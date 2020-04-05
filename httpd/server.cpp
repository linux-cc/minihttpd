#include "httpd/server.h"
#include "httpd/worker.h"
#include "httpd/connection.h"
#include "util/rbtree.h"
#include "util/rbtree_stack.h"
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
using util::RBTree;
using util::RBTreeT;

typedef RBTree<int, int>::Iterator Iter1;
typedef RBTreeT<int, int>::Iterator Iter2;

void printRBTree(RBTree<int, int> &rbt1, RBTreeT<int, int> &rbt2) {
    Iter1 it1 = rbt1.begin();
    for (Iter2 it2 = rbt2.begin(); it2 != rbt2.end(); it2++, it1++) {
        if (it2.value() != it1.value() || it2.color() != it1.color() || it1.key() != it2.key()) {
            printf("it2 not equals it: {%d, %d} <-> {%d, %d}\n", it2.value(), it2.color(), it1.value(), it1.color());
            abort();
        }
    }
}

#define INSERT(k, v)    rbt1.insert(k, v);rbt2.insert(k, v);printRBTree(rbt1, rbt2)
#define ASSIGN(k, v)    rbt1[k] = v;rbt2[k] = v;printRBTree(rbt1, rbt2)
#define ERASE(k)        rbt1.erase(k);rbt2.erase(k);printRBTree(rbt1, rbt2)

void testRBTree() {
    RBTree<int, int> rbt1;
    RBTreeT<int, int> rbt2;

    INSERT(50, 500);
    INSERT(30, 300);
    INSERT(45, 450);
    INSERT(40, 400);
    INSERT(20, 200);
    INSERT(25, 250);
    INSERT(35, 350);
    INSERT(80, 800);
    INSERT(85, 850);
    INSERT(90, 900);
    INSERT(95, 950);
    INSERT(65, 650);
    INSERT(60, 600);
    
    ERASE(35);
    ASSIGN(35, 350);
    ASSIGN(70, 700);
    ASSIGN(30, 333);
    ERASE(25);
    ERASE(95);
    ERASE(20);
    ERASE(50);
    ERASE(30);
    ERASE(40);
    ERASE(45);
    ERASE(35);
    ERASE(70);
    ERASE(80);
    ERASE(85);
    ERASE(90);
    ERASE(65);
    ERASE(60);
    
    int keys[1000];
    for (int i = 0; i < 1000; i++) {
        srand(i+1);
        int r = rand();
        INSERT(r, r);
        keys[i] = r;
    }
    for (int i = 0; i < 1000; i++) {
        ERASE(keys[i]);
    }
}

int main(int argc, char *argv[]) {
    memory::Allocater::createLocalKey();
    httpd::Server svr;
    svr.start("localhost", "9090");
    svr.run();
    memory::Allocater::deleteLocalKey();

    return 0;
}
