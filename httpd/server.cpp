#include "httpd/server.h"
#include "httpd/request.h"
#include "httpd/response.h"
#include "socket/addrinfo.h"
#include <math.h>

BEGIN_NS(httpd)

static volatile int __accept_lock = 0;
USING_CLASS(socket, EPollResult);
USING_CLASS(socket, Sockaddr);
USING_CLASS(socket, Peername);

bool Server::start(int workers, int maxClients) {
    if (!_server.create("localhost", "9090")) {
        _LOG_("server create error:%d:%s\n", errno, strerror(errno));
        return false;
    }
    _server.setNonblock();
    _workers = new Worker*[workers];
    int perMax = maxClients/workers;
    for (int i = 0; i < workers; ++i) {
        _workers[i] = new Worker(*this, perMax);
        _workers[i]->start();
    }
    _LOG_("server listen on port 9090\n");

    return true;
}

bool Worker::onInit() {
    if (!_poller.create(_maxClients)) {
        _LOG_("poller create false\n");
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
    if (_connsQ.size() <= round(_connsQ.capacity() * 2 / 10)) {
        disableAccept(holdLock);
        return;
    }
    bool locked = atomic_bool_cas(&__accept_lock, 0, 1); 
    if (locked) {
        if (!holdLock) {
            holdLock = true;
            int error = _poller.add(_server);
            if (error) {
                _LOG_("poll add server error: %d:%s\n", errno, strerror(errno));
            }
        }
    } else {
        disableAccept(holdLock);
    }
}

void Worker::disableAccept(bool &holdLock) {
    if (holdLock) {
        holdLock = false;
        int error = _poller.del(_server);
        if (error) {
            _LOG_("poll del server error: %d:%s\n", errno, strerror(errno));
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
            if (it->fd() == _server) {
                _LOG_("=======================enter onAccept========================\n");
                onAccept();
                continue;
            }
            if (holdLock) {
                _eventQ.pushBack(&*it);
            } else {
                onRequest(*it);
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
                _LOG_("accept error: %d:%s\n", errno, strerror(errno));
            }
            int error = _poller.mod(_server);
            if (error) {
                _LOG_("poll mod server error: %d:%s\n", errno, strerror(errno));
            }
            break;
        }
        TcpSocket client(fd);
        Connection *conn = _connsQ.popFront();
        if (!conn) {
            _LOG_("Connection is empty\n");
            client.close();
            break;
        }
        client.setNoDelay();
        client.setNonblock();
        conn->attach(fd);
        int error = _poller.add(fd, conn);
        if (error) {
            _LOG_("poll add client error: %d:%s\n", errno, strerror(errno));
        }

        Sockaddr addr;
        if (!client.getpeername(addr)) {
            _LOG_("Worker getpeername error: %d:%s\n", errno, strerror(errno));
        }
        Peername peer(addr);
        _LOG_("server accept: [%s|%d], fd: %d\n", (const char*)peer, peer.port(), fd);
    }
}

void Worker::onHandleEvent() {
    EPollEvent *event;
    while ((event = _eventQ.popFront())) {
        onRequest(*event);
    }
}

void Worker::onRequest(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    if (!conn->recv()) {
        close(conn);
        return;
    }
    Request &request = conn->request();
    const char *p1 = conn->pos();
    const char *p2 = strstr(p1, CRLF);
    if (p2) {
        switch(request.status()) {
        case Request::PROCESS_LINE:
            request.parseStatusLine(p1, p2);
            p1 = p2 + 2;
        case Request::PROCESS_HEADERS:
            while ((p2 = strstr(p1, CRLF))) {
                request.addHeader(p1, p2);
                p1 = p2 + 2;
                if (request.inProcessContent()) {
                    break;
                }
            }
            if (request.inProcessHeaders()) {
                break;
            }
        case Request::PROCESS_CONTENT:
            p1 += request.setContent(p1, conn->last());
            if (request.inProcessContent()) {
                break;
            }
        case Request::PROCESS_DONE:
            onResponse(conn, request);
            break;
        }
        conn->adjust(p1);
    }
    _poller.mod(*conn, conn);
}

bool Worker::onResponse(Connection *conn, Request &request) {
    _LOG_("fd: %d, Request headers:\n%s\n", (int)*conn, request.headers().c_str());
    Response response;
    response.parseRequest(request);
    request.reset();
    conn->sendn(response.headers().c_str(), response.headers().length());
    _LOG_("fd: %d, Response headers:\n%s\n", (int)*conn, response.headers().c_str());
    int fd = response.fd();
    if (fd > 0) {
        int length = response.contentLength();
        int n = sendFile(conn, fd, length);
        if (n != length) {
            _LOG_("senfile len: %d not equal file length: %d\n", n, length);
            close(conn);
            return false;
        }
    }
    if (response.connectionClose()) {
        close(conn);
        return false;
    }

    return true;
}

int Worker::sendFile(Connection *conn, int fd, off_t length) {
#ifdef __linux__
    return sendfile(*conn, fd, NULL, length);
#else
    off_t len = length;
    if (sendfile(fd, *conn, 0, &len, NULL, 0)) {
        _LOG_("sendfile error: %d:%s\n", errno, strerror(errno));
    }
    return len;
#endif
}

void Worker::close(Connection *conn) {
    int error = _poller.del(*conn);
    if (error) {
        _LOG_("poll del client fd: %d, error: %d:%s\n", (int)*conn, errno, strerror(errno));
    }
    conn->close();
    _connsQ.pushBack(conn);
}

END_NS

