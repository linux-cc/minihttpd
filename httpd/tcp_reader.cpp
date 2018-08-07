#include "httpd/tcp_reader.h"
#include "httpd/tcp_server.h"

BEGIN_NS(httpd)

USING_CLASS(socket, EPollResult);

bool TcpReader::onInit() {
    if (!_poller.create(_maxClients)) {
        return false;
    }
    if (!_server.create("localhost", "9090")) {
        return false;
    }
    _server.setNonblock();
    _poller.add(_server);

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

void TcpReader::onCancel() {
    _server.close();
    for (int i = 0; i < _maxClients; ++i) {
        _nodes[i].conn.release();
    }
    delete []_nodes;
}

void TcpReader::run() {
    while (!_quit) {
        EPollResult result = _poller.wait(100);
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            if (it->fd() == _server) {
                onAccept();
                continue;
            }
        }
    }
}

void TcpReader::onAccept() {
    while (true) {
        if (!_free) {
            break;
        } 
        TcpSocket client;
        if (!_server.accept(client)) {
#ifdef __DEBUG__
            if (_server.errcode() != EWOULDBLOCK && _server.errcode() != EAGAIN) {
                __LOG__("TcpReader onAccept error: %d:%s\n", _server.errcode(), _server.errinfo());
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
#ifdef __DEBUG__
        Sockaddr addr;
        if (!client.getpeername(addr)) {
            __LOG__("TcpReader getpeername error: %d:%s\n", client.errcode(), client.errinfo());
        }
        Peername peer(addr);
        __LOG__("server accept: [%s|%d]\n", (const char*)peer, peer.port());
#endif
    }
}

END_NS
