#include "util/config.h"
#include "util/atomic.h"
#include "httpd/worker.h"
#include "httpd/server.h"
#include "httpd/request.h"
#include "httpd/response.h"
#include "httpd/connection.h"
#include "network/addrinfo.h"
#include <math.h>
#include <errno.h>

namespace httpd {

using network::EPollResult;
using network::Sockaddr;
using network::Peername;

static int __accept_lock = 0;

bool Worker::onInit() {
    if (!_poller.create(MAX_WORKER_CONN)) {
        _LOG_("poller create false\n");
        return false;
    }

    return true;
}

bool Worker::tryLockAccept() {
    if (_actives >= round(MAX_WORKER_CONN * 2 / 10)) {
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
                    _eventList.push(&*it);
                } else {
                    onRequest(*it);
                }
            } else if (it->isPollOut()) {
                if (holdLock) {
                    _eventList.push(&*it);
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

        if (_connsList.size() >= MAX_WORKER_CONN) {
            _LOG_("Connection is empty\n");
            client.close();
            break;
        }
        client.setNoDelay();
        client.setNonblock();
        Connection *conn = memory::SimpleAlloc<Connection>::New(client);
        _connsList.push(conn);
        _server._eventQ.enqueue(Server::Item(conn));
        int error = _poller.add(client, conn);
        if (error) {
            _LOG_("poll add client error: %d:%s\n", errno, strerror(errno));
        }
        _LOG_("server accept: [%s|%d], fd: %d, Connection: %p\n", client.getPeerName().name(), client.getPeerName().port(), (int)client, conn);
    }
}

void Worker::onHandleEvent() {
    EPollEvent *event;
    while (_eventList.pop(event)) {
        if (event->isPollIn()) {
            onRequest(*event);
        } else if (event->isPollOut()) {
            onResponse(*event);
        } else {
            _LOG_("handle unknown event, fd: %d, connection: %p\n", event->fd(), event->data());
        }
    }
}

void Worker::onRequest(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    do {
        if (!conn->recv()) {
            close(conn);
            break;
        }
        
        _poller.mod(conn->fd(), conn);
        Request *request = conn->getRequest();
        if (!request->parseHeaders(conn)) {
            break;
        }
        
        if (request->hasContent()) {
            request->parseContent(conn);
        }
        if (request->isCompleted()) {
            _poller.mod(conn->fd(), conn, true);
            conn->setRequest(NULL);
        }
    } while (0);
    _server._eventQ.enqueue(Server::Item(conn));
}

void Worker::onResponse(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    Request request;// = conn->request();
    Response response;// = conn->response();
    switch(response.status()) {
    case Response::PARSE_REQUEST:
        response.parseRequest(request);
        //request.reset(response.is100Continue());
        _LOG_("fd: %d, Response headers:\n%s\n", conn->fd(), response.headers().data());
    case Response::SEND_HEADERS: {
        if (!response.sendHeaders(conn)) {
            _LOG_("Response send headers error: %d:%s\n", errno, strerror(errno));
            close(conn);
            return;
        }
        if (response.inSendHeaders()) {
            break;
        }
    }
    case Response::SEND_CONTENT:
        if (!response.sendContent(conn)) {
            _LOG_("Response send content error: %d:%s\n", errno, strerror(errno));
            close(conn);
            return;
        }
        if (response.inSendContent()) {
            break;
        }
    case Response::SEND_DONE:
        if (!conn->send()) {
            _LOG_("Response send buffer error: %d:%s\n", errno, strerror(errno));
            close(conn);
            return;
        }
        if (conn->sendCompleted()) {
            if (response.connectionClose()) {
                close(conn);
            }
            response.reset();
            _poller.mod(conn->fd(), conn);
            return;
        }
    }
    int error = _poller.mod(conn->fd(), conn, true);
    if (error) {
        _LOG_("onResponse poll mode: %d:%s\n", errno, strerror(errno));
    }
}

void Worker::close(Connection *conn) {
    _poller.del(conn->fd());
    _LOG_("close connection: %p, fd: %d\n", conn, conn->fd());
    conn->close();
    _connsList.erase(conn);
    memory::SimpleAlloc<Connection>::Delete(conn);
}

} /* namespace httpd */

