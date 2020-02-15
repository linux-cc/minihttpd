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

    return true;
}

void Worker::onCancel() {
    for (list<Connection*>::iterator it = _connsQ.begin(); it != _connsQ.end(); it++) {
        (*it)->release();
        delete *it;
    }
    _connsQ.clear();
}

bool Worker::tryLockAccept() {
    if (_actives >= round(_maxClients * 2 / 10)) {
        return false;
    }
    
    bool locked = util::atomicBoolCas(&__accept_lock, 0, 1);
    if (locked) {
        if (!_acceptLock) {
            _acceptLock = true;
            int error = _poller.add(_server);
            if (error) {
                _LOG_("poll add server error: %d:%s\n", errno, strerror(errno));
            }
        }
        return true;
    } else {
        if (_acceptLock) {
            _acceptLock = false;
            int error = _poller.del(_server);
            if (error) {
                _LOG_("poll del server error: %d:%s\n", errno, strerror(errno));
            }
        }
        return false;
    }
}

void Worker::unlockAccept() {
    if (_acceptLock) {
        util::atomicBoolCas(&__accept_lock, 1, 0);
    }
}

void Worker::run() {
    while (!_server.isQuit()) {
        bool holdLock = tryLockAccept();
        EPollResult result = _poller.wait(200);
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            if (it->isPollIn()) {
                if (it->fd() == _server) {
                    onAccept();
                } else if (holdLock) {
                    _eventQ.push_back(&*it);
                } else {
                    onRequest(*it);
                }
            } else if (it->isPollOut()) {
                if (holdLock) {
                    _eventQ.push_back(&*it);
                } else {
                    onResponse(*it);
                }
            }
        }
        unlockAccept();
        onHandleEvent();
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

        if (_connsQ.size() >= _maxClients) {
            _LOG_("Connection is empty\n");
            client.close();
            break;
        }
        client.setNoDelay();
        client.setNonblock();
        Connection *conn;
        if (_connsQ.empty()) {
            conn = new Connection;
        } else {
            conn = _connsQ.front();
            _connsQ.pop_front();
        }
        conn->attach(client);
        _server.update(conn, this);
        int error = _poller.add(client, conn);
        if (error) {
            _LOG_("poll add client error: %d:%s\n", errno, strerror(errno));
        }
        _LOG_("server accept: [%s|%d], fd: %d, Connection: %p\n", client.getPeerName().name(), client.getPeerName().port(), (int)client, conn);
    }
}

void Worker::onHandleEvent() {
    EPollEvent *event;
    while ((event = _eventQ.front())) {
        if (event->isPollIn()) {
            onRequest(*event);
        } else if (event->isPollOut()) {
            onResponse(*event);
        } else {
            _LOG_("handle unknown event\n");
        }
        _eventQ.pop_front();
    }
}

void Worker::onRequest(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    if (!conn->recv()) {
        closeInternal(conn);
        return;
    }
    _poller.mod(*conn, conn);
    Request request;
    size_t length = request.parse(conn->begin(), conn->end());
    if (!length) {
        return;
    }
    conn->seek(length);
    if (request.isComplete()) {
        _poller.mod(*conn, conn, true);
    }
    _server.update(conn, this);
}

void Worker::onResponse(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    Request request;// = conn->request();
    Response response;// = conn->response();
    switch(response.status()) {
    case Response::PARSE_REQUEST:
        response.parseRequest(request);
        //request.reset(response.is100Continue());
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
    _connsQ.push_back(conn);
}

void Worker::closeInternal(Connection *conn) {
    close(conn);
    _server.remove(conn, this);
}

} /* namespace httpd */

