#include "httpd/server.h"
#include "httpd/worker.h"
#include "httpd/connection.h"
#include "util/rbtree.h"
#include <errno.h>
#include <stdio.h>

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

        List<Item> &newList = _timeoutQ.at(item._newIdx);
        if (!newList.find(item)) {
            newList.pushBack(item);
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
            List<Item> &curList = _timeoutQ.at(_curIndex);
            while (!curList.empty()) {
                Item &item = curList.front();
                _LOG_("timeout fd: %d, old: %d, new: %d, closed: %d", item._fd, item._oldIdx, item._newIdx, item._closed);
                TcpSocket(item._fd).close();
                curList.popFront();
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
    using util::RBTree;
    //memory::Allocater::createLocalKey();
    //httpd::Server svr;
    //svr.start("localhost", "9090");
    //svr.run();
    //memory::Allocater::deleteLocalKey();
    typedef RBTree<int, int>::Iterator Iter;
    RBTree<int, int> rbt, rbt2;
    Iter it0;
    it0 = rbt.insert(50, 500);
    it0 = rbt.insert(30, 300);
    it0 = rbt.insert(45, 450);
    it0 = rbt.insert(40, 400);
    it0 = rbt.insert(20, 200);
    it0 = rbt.insert(25, 250);
    it0 = rbt.insert(35, 350);
    it0 = rbt.insert(80, 800);
    it0 = rbt.insert(85, 850);
    it0 = rbt.insert(90, 900);
    it0 = rbt.insert(95, 950);
    it0 = rbt.insert(65, 650);
    it0 = rbt.insert(60, 600);
    it0 = rbt2.insert2(50, 500);
    it0 = rbt2.insert2(30, 300);
    it0 = rbt2.insert2(45, 450);
    it0 = rbt2.insert2(40, 400);
    it0 = rbt2.insert2(20, 200);
    it0 = rbt2.insert2(25, 250);
    it0 = rbt2.insert2(35, 350);
    it0 = rbt2.insert2(80, 800);
    it0 = rbt2.insert2(85, 850);
    it0 = rbt2.insert2(90, 900);
    it0 = rbt2.insert2(95, 950);
    it0 = rbt2.insert2(65, 650);
    it0 = rbt2.insert2(60, 600);
    //rbt[100] = 101;
    //rbt2[100] = 101;
    for (Iter it = rbt.begin(); it != rbt.end(); it++) {
        printf("key: %d, value: %d\n", it.key(), it.value());
        Iter it2 = rbt2.find(it.key());
        if (it2 == rbt2.end()) {
            printf("rbt2 not found: %d\n", it.key());
            continue;
        }
        if (it2.value() != it.value() || it2.color() != it.color()) {
            printf("it2 not equals it: {%d, %d} <-> {%d, %d}\n", it2.value(), it2.color(), it.value(), it.color());
        } else {
            printf("it2 equals it: {%d, %d} <-> {%d, %d}\n", it2.value(), it2.color(), it.value(), it.color());
        }
    }

    return 0;
}
