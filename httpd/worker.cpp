#include "httpd/worker.h"
#include "httpd/server.h"
#include "httpd/request.h"
#include "httpd/response.h"
#include "httpd/connection.h"
#include "socket/addrinfo.h"
#include <math.h>

BEGIN_NS(httpd)

static volatile int __accept_lock = 0;
USING_CLASS(socket, EPollResult);
USING_CLASS(socket, Sockaddr);
USING_CLASS(socket, Peername);

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
        EPollResult result = _poller.wait(200);
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            if (it->isPollIn()) {
                if (it->fd() == _server) {
                    onAccept();
                    continue;
                }
                if (holdLock) {
                    _eventQ.pushBack(&*it);
                } else {
                    onRequest(*it);
                }
            } else if (it->isPollOut()) {
                if (holdLock) {
                    _eventQ.pushBack(&*it);
                } else {
                    onResponse(*it);
                }
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
        _server.update(conn, this);
        int error = _poller.add(fd, conn);
        if (error) {
            _LOG_("poll add client error: %d:%s\n", errno, strerror(errno));
        }

        Sockaddr addr;
        if (!client.getpeername(addr)) {
            _LOG_("Worker getpeername error: %d:%s\n", errno, strerror(errno));
        }
        Peername peer(addr);
        _LOG_("server accept: [%s|%d], fd: %d, Connection: %p\n", (const char*)peer, peer.port(), fd, conn);
    }
}

void Worker::onHandleEvent() {
    EPollEvent *event;
    while ((event = _eventQ.popFront())) {
        if (event->isPollIn()) {
            onRequest(*event);
        } else if (event->isPollOut()) {
            onResponse(*event);
        } else {
            _LOG_("handle unknown event\n");
        }
    }
}

void Worker::onRequest(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    if (!conn->recv()) {
        _LOG_("onRequest recv error: %s\n", strerror(errno));
        closeInternal(conn);
        return;
    }
    _poller.mod(*conn, conn);
    Request &request = conn->request();
    const char *pos = conn->pos();
    const char *last = conn->last();
    switch(request.status()) {
    case Request::PARSE_LINE:
        pos += request.parseStatusLine(pos, last);
        if (request.inParseStatusLine()) {
            break;
        }
    case Request::PARSE_HEADERS:
        pos += request.parseHeaders(pos, last);
        if (request.inParseHeaders()) {
            break;
        }
        _LOG_("fd: %d, Request headers:\n%s", (int)*conn, request.headers().c_str());
    case Request::PARSE_CONTENT:
        pos += request.parseContent(pos, last);
        if (request.inParseContent()) {
            break;
        }
    case Request::PARSE_DONE:
        _poller.mod(*conn, conn, true);
        _LOG_("fd: %d, Request content:\n%s\n", (int)*conn, request.content().c_str());
    }
    conn->seek(pos);
    _server.update(conn, this);
}

void Worker::onResponse(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    Request &request = conn->request();
    Response &response = conn->response();
    switch(response.status()) {
    case Response::PARSE_REQUEST:
        response.parseRequest(request);
        request.reset(response.is100Continue());
        _LOG_("fd: %d, Response headers:\n%s\n", (int)*conn, response.headers());
    case Response::SEND_HEADERS: {
        if (!response.sendHeaders(conn)) {
            _LOG_("Response send headers error: %d:%s\n", errno, strerror(errno));
            closeInternal(conn);
            return;
        }
        if (response.inSendHeaders()) {
            break;
        }
    }
    case Response::SEND_CONTENT:
        if (!response.sendContent(conn)) {
            _LOG_("Response send content error: %d:%s\n", errno, strerror(errno));
            closeInternal(conn);
            return;
        }
        if (response.inSendContent()) {
            break;
        }
    case Response::SEND_DONE:
        if (!conn->send()) {
            _LOG_("Response send buffer error: %d:%s\n", errno, strerror(errno));
            closeInternal(conn);
            return;
        }
        if (!conn->needPollOut()) {
            if (response.connectionClose()) {
                closeInternal(conn);
            }
            response.reset();
            _poller.mod(*conn, conn);
            return;
        }
    }
    int error = _poller.mod(*conn, conn, true);
    if (error) {
        _LOG_("onResponse poll mode: %d:%s\n", errno, strerror(errno));
    }
}

void Worker::close(Connection *conn) {
    _poller.del(*conn);
    _LOG_("close connection: %p, fd: %d\n", conn, (int)*conn);
    conn->close();
    _connsQ.pushBack(conn);
}

void Worker::closeInternal(Connection *conn) {
    close(conn);
    _server.remove(conn, this);
}

END_NS

