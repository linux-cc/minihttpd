#include "httpd/server.h"
#include "httpd/worker.h"
#include "httpd/connection.h"
#include <errno.h>

namespace httpd {

Server::Server():
_timeoutQ(CONN_TIMEOUT),
_eventQ(MAX_WORKER * MAX_WORKER_CONN),
_curIndex(0),
_quit(false) {
    for (int i = 0; i < MAX_WORKER; ++i) {
        _workers[i] = memory::SimpleAlloc<Worker>::New(*this);
    }
}

Server::~Server() {
    for (int i = 0; i < MAX_WORKER; ++i) {
        memory::SimpleAlloc<Worker>::Delete(_workers[i]);
    }
}

bool Server::start(const char *host, const char *service) {
    if (!_server.create(host, service)) {
        _LOG_("server create error:%d:%s", errno, strerror(errno));
        return false;
    }
    _server.setNonblock();
    
    for (int i = 0; i < MAX_WORKER; ++i) {
        if (!_workers[i]->start()) {
            _LOG_("server start worker error:%d:%s", errno, strerror(errno));
            return false;
        }
    }
    _LOG_("server listen on %s:%s", host, service);

    return true;
}

void Server::update(Connection *conn, bool closed) {
    if (!closed) {
        conn->setOldIdx(conn->getNewIdx());
        int newIdx = _curIndex - 1;
        if (newIdx < 0) {
            newIdx = _timeoutQ.capacity() - 1;
        }
        conn->setNewIdx(newIdx);
    }
    _eventQ.enqueue(Item(conn, closed));
}

void Server::update() {
    Item item;
    while (_eventQ.dequeue(item)) {
        if (item._oldIdx != -1) {
            int n = _timeoutQ.at(item._oldIdx).remove(item);
            _LOG_("timeout remove %d item: { %d, %d, %d, %d }", n, item._fd, item._oldIdx, item._newIdx, item._closed);
        }
        if (item._closed) {
            if (item._newIdx != item._oldIdx) {
                int n = _timeoutQ.at(item._newIdx).remove(item);
                _LOG_("timeout remove %d item: { %d, %d, %d, %d }", n, item._fd, item._oldIdx, item._newIdx, item._closed);
            }
            continue;
        }

        SimpleList<Item> &newList = _timeoutQ.at(item._newIdx);
        if (!newList.find(item)) {
            newList.push_back(item);
            _LOG_("timeout push_back item: { %d, %d, %d, %d }", item._fd, item._oldIdx, item._newIdx, item._closed);
        }
    }
}

void Server::run() {
    struct timeval tv = { 0, 50000 };
    int tick = 0;
    while (!_quit) {
        update();
        if (++tick == 20) {
            SimpleList<Item> &curList = _timeoutQ.at(_curIndex);
            while (!curList.empty()) {
                Item &item = curList.front();
                _LOG_("timeout fd: %d, old: %d, new: %d, closed: %d", item._fd, item._oldIdx, item._newIdx, item._closed);
                TcpSocket(item._fd).close();
                curList.pop_front();
            }
            tick = 0;
            if (++_curIndex == _timeoutQ.capacity()) {
                _curIndex = 0;
            }
        }
        select(0, NULL, NULL, NULL, &tv);
    }
}

Server::Item::Item(Connection *conn, bool closed) {
    if (conn) {
        _fd = conn->fd();
        _newIdx = conn->getNewIdx();
        _oldIdx = conn->getOldIdx();
        _closed = closed;
        _LOG_("enqueue item: { %d, %d, %d, %d }", _fd, _oldIdx, _newIdx, _closed);
    } else {
        _fd = _newIdx = _oldIdx = -1;
        _closed = 0;
    }
}

} /* namespace httpd */

int main(int argc, char *argv[]) {
    memory::Allocater::createLocalKey();
    httpd::Server svr;
    svr.start("localhost", "9090");
    svr.run();
    memory::Allocater::deleteLocalKey();
    
    return 0;
}
