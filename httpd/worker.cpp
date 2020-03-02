#include "util/config.h"
#include "util/atomic.h"
#include "util/scoped_ptr.h"
#include "httpd/worker.h"
#include "httpd/server.h"
#include "httpd/request.h"
#include "httpd/response.h"
#include "httpd/connection.h"
#include "network/addrinfo.h"
#include <math.h>
#include <errno.h>

namespace httpd {

using util::ScopedPtr;
using memory::SimpleAlloc;
using network::EPollResult;
using network::Sockaddr;
using network::Peername;

static int __accept_lock = 0;

bool Worker::onInit() {
    if (!_poller.create(MAX_WORKER_CONN)) {
        _LOG_("poller create false");
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
                _LOG_("poll add server error: %d:%s", errno, strerror(errno));
            }
        }
        return true;
    } else {
        if (_acceptLock) {
            _acceptLock = false;
            int error = _poller.del(_server);
            if (error) {
                _LOG_("poll del server error: %d:%s", errno, strerror(errno));
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
        EPollResult result = _poller.wait(1000);
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
                _LOG_("accept error: %d:%s", errno, strerror(errno));
            }
            int error = _poller.mod(_server);
            if (error) {
                _LOG_("poll mod server error: %d:%s", errno, strerror(errno));
            }
            break;
        }

        if (_connsList.size() >= MAX_WORKER_CONN) {
            _LOG_("Connection is empty");
            client.close();
            break;
        }
        client.setNoDelay();
        client.setNonblock();
        Connection *conn = SimpleAlloc<Connection>::New(client);
        _connsList.push(conn);
        _server._eventQ.enqueue(Server::Item(conn));
        int error = _poller.add(client, conn);
        if (error) {
            _LOG_("poll add client error: %d:%s", errno, strerror(errno));
        }
        _LOG_("server accept: [%s|%d], fd: %d, Connection: %p", client.getPeerName().name(), client.getPeerName().port(), (int)client, conn);
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
            _LOG_("handle unknown event, fd: %d, connection: %p", event->fd(), event->data());
        }
    }
}

void Worker::onRequest(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    
    if (!conn->recv()) {
        close(conn);
        _server._eventQ.enqueue(Server::Item(conn));
        return;
    }
    bool write = false;
    while (true) {
        Request *last = conn->getRequest();
        ScopedPtr<Request> req(last ? last : SimpleAlloc<Request>::New(conn));
        if (req->inParseHeaders()) {
            if (!req->parseHeaders()) {
                break;
            }
            _LOG_("fd: %d, Request headers:\n%s", conn->fd(), req->headers().data());
            if (req->is100Continue()) {
                conn->addRequest(SimpleAlloc<Request>::New(*req));
                conn->setRequest(req.release());
                write = true;
                break;
            }
        }
        if (req->inParseContent()) {
            req->parseContent();
            if (req->isCompleted()) {
                conn->addRequest(req.release());
                write = true;
            } else {
                conn->setRequest(req.release());
                break;
            }
        }
    }
    _poller.mod(conn->fd(), conn, write);
    _server._eventQ.enqueue(Server::Item(conn));
}

void Worker::onResponse(EPollEvent &event) {
    Connection *conn = (Connection*)event.data();
    
    while (true) {
        ScopedPtr<Response> last(conn->getResponse());
        Response _new(conn);
        Response *resp = last.get() ? last.get() : &_new;
        
        if (!last) {
            Request *req = conn->popRequest();
            if (!req) break;
            resp->parseRequest(req);
            SimpleAlloc<Request>::Delete(req);
        }
        if (!resp->sendResponse()) {
            _LOG_("sendResponse error: %d:%s", errno, strerror(errno));
            close(conn);
            break;
        }
        if (resp->sendCompleted()) {
            if (resp->connectionClose()) {
                close(conn);
                break;
            }
        } else {
            conn->setResponse(resp == last ? last.release() : SimpleAlloc<Response>::New(*resp));
        }
    }
    _poller.mod(conn->fd(), conn);
}

void Worker::close(Connection *conn) {
    _poller.del(conn->fd());
    _LOG_("close connection: %p, fd: %d", conn, conn->fd());
    conn->close();
    _connsList.erase(conn);
    memory::SimpleAlloc<Connection>::Delete(conn);
}

} /* namespace httpd */

