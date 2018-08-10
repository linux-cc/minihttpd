#include "httpd/server.h"
#include "httpd/request.h"
#include "socket/addrinfo.h"

BEGIN_NS(httpd)

static volatile int __accept_lock = 0;
USING_CLASS(socket, EPollResult);
USING_CLASS(socket, Sockaddr);
USING_CLASS(socket, Peername);

bool Server::start(int workers, int maxClients) {
    if (!_server.create("localhost", "9090")) {
        __LOG__("server create error:%d:%s\n", errno, strerror(errno));
        return false;
    }

    __LOG__("server listen on port 9090\n");
    _server.setNonblock();
    _workers = new Worker*[workers];
    int perMax = maxClients/workers;
    for (int i = 0; i < workers; ++i) {
        _workers[i] = new Worker(*this, perMax);
        _workers[i]->start();
    }

    return true;
}

bool Worker::onInit() {
    if (!_poller.create(_maxClients)) {
        __LOG__("worker poller create false\n");
        return false;
    }

    _conns = new Connection[_maxClients];
    _connsQ.init(_conns, _maxClients);
    _eventQ.init(NULL, _maxClients);

    return true;
}

void Worker::onCancel() {
    for (int i = 0; i < _maxClients; ++i) {
        _conns[i].release();
    }
    delete []_conns;
}

void Worker::tryLockAccept(bool &holdLock) {
    bool locked = atomic_bool_cas(&__accept_lock, 0, 1); 
    if (locked) {
        if (!holdLock) {
            holdLock = true;
            _poller.add(_server);
        }
    } else {
        if (holdLock) {
            holdLock = false;
            _poller.del(_server);
        }
    }
}

void Worker::unlockAccept() {
    atomic_bool_cas(&__accept_lock, 1, 0);
}

void Worker::run() {
    bool holdLock = false;
    while (!_server.isQuit()) {
        tryLockAccept(holdLock);
        EPollResult result = _poller.wait(100);
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            if (it->events() & EPOLLIN) {
                if (it->fd() == _server) {
                    __LOG__("=======================enter onAccept========================\n");
                    onAccept();
                    continue;
                }
                if (holdLock) {
                    _eventQ.pushBack(&*it);
                } else {
                    onRequest(*it);
                }
            } else if (it->events() & EPOLLOUT) {

            } else {
                __LOG__("unknown event happen\n");
            }
        }
        if (holdLock) {
            unlockAccept();
            onHandleEvent();
        }
    }
}

void Worker::onAccept() {
    while (true) {
        int fd = _server.accept();
        if (fd < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                __LOG__("Worker onAccept error: %d:%s\n", errno, strerror(errno));
            }
            _poller.mod(_server);
            break;
        }
        Connection *conn = _connsQ.popFront();
        if (!conn) {
            __LOG__("Worker onAccept Connection empty");
            break;
        }
        conn->attach(fd);
        _poller.add(fd, conn);
        __LOG__("onAccept, Connection:%p, fd:%d\n", conn, fd);

        Sockaddr addr;
        if (!TcpSocket(fd).getpeername(addr)) {
            __LOG__("Worker getpeername error: %d:%s\n", errno, strerror(errno));
        }
        Peername peer(addr);
        __LOG__("server accept: [%s|%d]\n", (const char*)peer, peer.port());
    }
}

void Worker::onHandleEvent() {
    while (!_eventQ.empty()) {
        onRequest(*_eventQ.popFront());
        sleep(1);
    }
}

void Worker::onRequest(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    printf("[%ld]onRequest:%p\n", (intptr_t)pid(), conn);

    Request request;
    if (!readHeader(conn, request)) {
        close(conn);
        return;
    }
    if (!readContent(conn, request)) {
        close(conn);
        return;
    }
    onResponse(conn, request);
    close(conn);
}

bool Worker::readHeader(Connection *conn, Request &request) {
    char buf[1024];
    int len = conn->recvline(buf, sizeof(buf));
    if (len <= 0) {
        __LOG__("worker readHeader read status line error:%d:%s, len:%d\n",
            errno, strerror(errno), len);
        return false;
    }
    request.parseStatusLine(buf);
    while (true) {
        len = conn->recvline(buf, sizeof(buf));
        if (len <= 0) {
            __LOG__("worker readHeader read header error:%d:%s, len:%d\n",
                errno, strerror(errno), len);
            return false;
        }
        Header header(buf);
        if (header.empty()) {
            __LOG__("worker readHeader read end\n");
            break;
        }
        request.addHeader(header);
        __LOG__("worker readHeader: %s\n", header.toString().c_str());
    }
    
    return true;
}

bool Worker::readContent(Connection *conn, Request &request) {
    int length = request.contentLength();
    __LOG__("worker readContent length: %d\n", length);
    if (length) {
        int read = conn->recvn(request.content(), length);
        if (read <= 0) {
            return false;;
        }
        request.decodeContent();
    }
    
    return true;
}

void Worker::onResponse(Connection *conn, Request &request) {
    string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Server: myframe httpd 1.0\r\n";
    response += "\r\n";
    response += "<HTML><TITLE>Hello Httpd</TITLE>\r\n";
    response += "<BODY><P>Congratulations Hello World!!!</P></BODY></HTML>\r\n";
    conn->sendn(response.c_str(), response.size());
}

void Worker::close(Connection *conn) {
    _poller.del(*conn);
    conn->close();
    _connsQ.pushBack(conn);
}

END_NS

