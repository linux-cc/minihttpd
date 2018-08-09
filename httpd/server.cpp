#include "httpd/server.h"
#include "httpd/request.h"
#include "socket/addrinfo.h"
#include <errno.h>

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
    }
    _nodes[i - 1].next = NULL;

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
        disableAccept();
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            if (it->fd() == _server) {
                onAccept();
                continue;
            }
            onRequest(*it);
        }
    }
}

void Worker::onAccept() {
    while (true) {
        if (!_free) {
            break;
        } 
        int fd = _server.accept();
        if (fd < 0) {
#ifdef _DEBUG_
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                __LOG__("Worker onAccept error: %d:%s\n", errno, strerror(errno));
            }
#endif
            _poller.mod(_server);
            break;
        }
        Node *n = _free;
        n->conn.attach(fd);
        _free = _free->next;
        _poller.add(fd, n);
#ifdef _DEBUG_
        Sockaddr addr;
        if (!TcpSocket(fd).getpeername(addr)) {
            __LOG__("Worker getpeername error: %d:%s\n", errno, strerror(errno));
        }
        Peername peer(addr);
        __LOG__("server accept: [%s|%d]\n", (const char*)peer, peer.port());
#endif
    }
}

void Worker::onRequest(EPollEvent &event) {
    Node *n = (Node*)event.data();
    char buf[1024];
    int len = n->conn.recvline(buf, sizeof(buf));
    if (len <= 0) {
        close(n);
        return;
    }
    Request request;
    request.parseStatusLine(buf);
    while (true) {
        len = n->conn.recvline(buf, sizeof(buf));
        if (len <= 0) {
            close(n);
            break;
        }
        Header header(buf);
        if (header.empty()) {
            break;
        }
    }
}

void Worker::close(Node *node) {
#ifdef _DEBUG_
        __LOG__("Worker onPollIn error:%d:%s, fd:%d\n", errno, strerror(errno), (int)node->conn);
#endif
    node->conn.close();
    node->next = _free;
    _free = node;
}

END_NS
