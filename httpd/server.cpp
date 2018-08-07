#include "httpd/server.h"
#include "socket/addrinfo.h"

BEGIN_NS(httpd)

static volatile int __accept_lock = 0;
USING_CLASS(socket, EPollResult);
USING_CLASS(socket, Sockaddr);
USING_CLASS(socket, Peername);

bool Server::onInit() {
    if (!_server.create("localhost", "9090")) {
        return false;
    }
    _server.setNonblock();

    return true;
}


Worker::Worker(Server &server, int maxClients):
_server(server),
_free(NULL),
_nodes(NULL),
_maxClients(maxClients),
_quit(false),
_holdLock(false) {

}

bool Worker::onInit() {
    if (!_poller.create(_maxClients)) {
        return false;
    }

    _free = _nodes = new Node[_maxClients];
    int i = 1;
    for (; i < _maxClients; ++i) {
        _nodes[i - 1].next = &_nodes[i];
        _nodes[i - 1].conn.init();
    }
    _nodes[i - 1].next = NULL;
    _nodes[i - 1].conn.init();

    return true;
}

void Worker::onCancel() {
    for (int i = 0; i < _maxClients; ++i) {
        _nodes[i].conn.release();
    }
    delete []_nodes;
}

void Worker::enableAccept() {
    if (!_holdLock) {
        int id = *(int*)pid();
        bool lock = atomic_bool_cas(&__accept_lock, 0, id); 
        if (lock) {
            _holdLock = true;
            _poller.add(_server);
        }
    }
}

void Worker::disableAccept() {
    if (_holdLock) {
        int id = *(int*)pid();
        bool unlock = atomic_bool_cas(&__accept_lock, id, 0);
        if (unlock) {
            _holdLock = false;
            _poller.del(_server);
        }
    }
}

void Worker::run() {
    while (!_quit) {
        enableAccept();
        EPollResult result = _poller.wait(100);
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            if (it->fd() == _server && _holdLock) {
                onAccept();
                disableAccept();
                continue;
            }
            onPollIn(*it);
        }
    }
}

void Worker::onAccept() {
    while (true) {
        if (!_free) {
            break;
        } 
        TcpSocket client;
        if (!_server.accept(client)) {
#ifdef _DEBUG_
            if (_server.errcode() != EWOULDBLOCK && _server.errcode() != EAGAIN) {
                __LOG__("Worker onAccept error: %d:%s\n", _server.errcode(), _server.errinfo());
            }
#endif
            _poller.mod(_server);
            break;
        }
        client.setNonblock();
        Connection *conn = &_free->conn;
        conn->attach(client);
        _free = _free->next;
        _poller.add(client, conn);
#ifdef _DEBUG_
        Sockaddr addr;
        if (!client.getpeername(addr)) {
            __LOG__("Worker getpeername error: %d:%s\n", client.errcode(), client.errinfo());
        }
        Peername peer(addr);
        __LOG__("server accept: [%s|%d]\n", (const char*)peer, peer.port());
#endif
    }
}

void Worker::onPollIn(EPollEvent &event) {

}

END_NS
