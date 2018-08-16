#include "httpd/worker.h"
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
            int error = _poller.addPollIn(_server);
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
        int error = _poller.delPollIn(_server);
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
            int error = _poller.modPollIn(_server);
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
        int error = _poller.addPollIn(fd, conn);
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
        closeInternal(conn);
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
            _LOG_("fd: %d, Request headers:\n%s", (int)*conn, request.headers().c_str());
        case Request::PROCESS_CONTENT:
            p1 += request.setContent(p1, conn->last());
            if (request.inProcessContent()) {
                break;
            }
        case Request::PROCESS_DONE:
            _LOG_("fd: %d, Request content:\n%s\n", (int)*conn, request.content());
            _poller.addPollOut(*conn, conn);
        }
        conn->adjust(p1);
    }
    _poller.modPollIn(*conn, conn);
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
        _LOG_("fd: %d, Response headers:\n%s\n", (int)*conn, response.originHeaders());
    case Response::SEND_HEADERS: {
        int length = response.headersLength();
        int n = conn->send(response.headers(), length);
        if (n < 0) {
            _LOG_("Response send headers error: %d:%s\n", errno, strerror(errno));
            closeInternal(conn);
            return;
        }
        response.addHeadersPos(n + 1);
        if (response.inSendHeaders()) {
            break;
        }
    }
    case Response::SEND_CONTENT:
        if (!sendFile(conn, response)) {
            _LOG_("Response sendfile error: %d:%s\n", errno, strerror(errno));
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
            _poller.delPollOut(*conn);
            return;
        }
    }
    _poller.modPollOut(*conn, conn);
}

bool Worker::sendFile(Connection *conn, Response &response) {
    int fd = response.fileFd();
    off_t length = response.contentLength();
    if (fd < 0 || length == 0) {
        return true;
    }
    off_t pos = response.filePos();
    off_t n = length;
#ifdef __linux__
    n = sendfile(*conn, fd, &pos, length);
    if (n < 0) {
        return errno != EAGAIN ? false : true;
    }
#else
    if (sendfile(fd, *conn, pos, &n, NULL, 0)) {
        return errno != EAGAIN ? false : true;
    }
    response.addFilePos(n + 1);
    _LOG_("sendfile n:%lld, length:%lld\n", n, length);
#endif
    return true;
}

void Worker::close(Connection *conn) {
    int error = _poller.delPollIn(*conn);
    _LOG_("close connection: %p, fd: %d, poll del ret: %d\n", conn, (int)*conn, error);
    conn->close();
    _connsQ.pushBack(conn);
}

void Worker::closeInternal(Connection *conn) {
    close(conn);
    _server.remove(conn, this);
}

END_NS

