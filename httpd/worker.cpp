#include "config.h"
#include "httpd/worker.h"
#include "httpd/server.h"
#include "httpd/request.h"
#include "httpd/response.h"
#include "httpd/connection.h"
#include "network/addrinfo.h"
#include "util/atomic.h"
#include <math.h>

namespace httpd {

using network::EPollResult;
using network::Sockaddr;
using network::Peername;

static int __accept_lock = 0;

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
    bool locked = util::atomicBoolCas(&__accept_lock, 0, 1); 
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
    util::atomicBoolCas(&__accept_lock, 1, 0);
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
        TcpSocket client(_server.accept());
        if (client < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                _LOG_("accept error: %d:%s\n", errno, strerror(errno));
            }
            int error = _poller.mod(_server);
            if (error) {
                _LOG_("poll mod server error: %d:%s\n", errno, strerror(errno));
            }
            break;
        }
        Connection *conn = _connsQ.popFront();
        if (!conn) {
            _LOG_("Connection is empty\n");
            client.close();
            break;
        }
        client.setNoDelay();
        client.setNonblock();
        conn->attach(client);
        _server.update(conn, this);
        int error = _poller.add(client, conn);
        if (error) {
            _LOG_("poll add client error: %d:%s\n", errno, strerror(errno));
        }

        Sockaddr addr;
        if (!client.getpeername(addr)) {
            _LOG_("Worker getpeername error: %d:%s\n", errno, strerror(errno));
        }
        Peername peer(addr);
        _LOG_("server accept: [%s|%d], fd: %d, Connection: %p\n", (const char*)peer, peer.port(), (int)client, conn);
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
    _poller.mod(*conn, conn);
    Request &request = conn->request();
    const char *begin = conn->begin();
    const char *end = conn->end();
    switch(request.status()) {
    case Request::PARSE_LINE:
        begin += request.parseStatusLine(begin, end);
        if (request.inParseStatusLine()) {
            break;
        }
    case Request::PARSE_HEADERS:
        begin += request.parseHeaders(begin, end);
        if (request.inParseHeaders()) {
            break;
        }
        _LOG_("fd: %d, Request headers:\n%s", (int)*conn, request.headers().c_str());
    case Request::PARSE_CONTENT:
        begin += request.parseContent(begin, end);
        if (request.inParseContent()) {
            break;
        }
    case Request::PARSE_DONE:
        _poller.mod(*conn, conn, true);
        _LOG_("fd: %d, Request content:\n%s\n", (int)*conn, request.content().c_str());
    }
    conn->seek(begin);
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
        _LOG_("fd: %d, Response headers:\n%s\n", (int)*conn, response.headers().c_str());
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

} /* namespace httpd */

