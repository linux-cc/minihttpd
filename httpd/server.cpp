#include "httpd/server.h"
#include "httpd/worker.h"
#include "httpd/connection.h"
#include "util/rbtree.h"
#include "util/rbtree_stack.h"
#include "util/map.h"
#include "util/set.h"
#include <errno.h>
#include <stdio.h>
#include <map>
#include <set>

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

template <typename T>
struct Identity {
    const T &operator()(const T &data) { return data; }
};

typedef RBTree<int, int, Identity<int> > RBTree1;
typedef RBTreeT<int, int, Identity<int> > RBTree2;
typedef RBTree1::Iterator Iter1;
typedef RBTree2::Iterator Iter2;

void printRBTree(RBTree1 &rbt1, RBTree2 &rbt2) {
    Iter1 it1 = rbt1.begin();
    for (Iter2 it2 = rbt2.begin(); it2 != rbt2.end(); it2++, it1++) {
        if (*it2 != *it1 || it2.color() != it1.color()) {
            printf("it2 not equals it: {%d, %d} <-> {%d, %d}\n", *it2, it2.color(), *it1, it1.color());
            abort();
        }
    }
}

#define INSERT(k)    rbt1.insert(k, false);rbt2.insert(k, false);printRBTree(rbt1, rbt2)
#define ERASE(k)     rbt1.erase(k);rbt2.erase(k);printRBTree(rbt1, rbt2)

void testRBTree() {
    RBTree1 rbt1;
    RBTree2 rbt2;

    INSERT(50);
    INSERT(30);
    INSERT(45);
    INSERT(40);
    INSERT(20);
    INSERT(25);
    INSERT(35);
    INSERT(80);
    INSERT(85);
    INSERT(90);
    INSERT(95);
    INSERT(65);
    INSERT(60);
    
    ERASE(25);
    ERASE(95);
    ERASE(20);
    ERASE(50);
    ERASE(30);
    ERASE(40);
    ERASE(45);
    ERASE(35);
    ERASE(80);
    ERASE(85);
    ERASE(90);
    ERASE(65);
    ERASE(60);
    
    int keys[1000];
    for (int i = 0; i < 1000; i++) {
        srand(i+1);
        int r = rand();
        INSERT(r);
        keys[i] = r;
    }
    for (int i = 0; i < 1000; i++) {
        ERASE(keys[i]);
    }
}

typedef std::map<int, int> StdMap;
typedef util::Map<int, int> MyMap;
typedef StdMap::iterator StdIt;
typedef MyMap::Iterator MyIt;

void diffMap(StdMap &m1, MyMap &m2) {
    MyIt it2 = m2.begin();
    for (StdIt it1 = m1.begin(); it1 != m1.end(); it1++, it2++) {
        if (it1->first != it2->first || it1->second != it2->second) {
            printf("diff: {%d, %d} -> {%d, %d}\n", it1->first, it1->second, it2->first, it2->second);
            abort();
        }
    }
}

void testMap() {
    StdMap m1;
    MyMap m2;
    
    int keys[10000];
    for (int i = 0; i < 10000; i++) {
        srand(i+1);
        int k = rand();
        keys[i] = k;
        m1.insert(std::make_pair(k, k));
        m2.insert(util::makePair(k, k));
        diffMap(m1, m2);
    }
    for (int i = 0; i < 10000; i++) {
        int k = keys[i];
        m1.erase(k);
        m2.erase(k);
        diffMap(m1, m2);
    }
    for (int i = 0; i < 10000; i++) {
        srand(10000 + i+1);
        int k = rand();
        keys[i] = k;
        m1[k] = k;
        m2[k] = k;
        diffMap(m1, m2);
    }
}

void diffSet(std::set<int> &s1, util::Set<int> &s2) {
    util::Set<int>::Iterator it2 = s2.begin();
    for (std::set<int>::iterator it1 = s1.begin(); it1 != s1.end(); it1++, it2++) {
        if (*it1 != *it2) {
            printf("diff: {%d} -> {%d}\n", *it1, *it2);
            abort();
        }
    }
}

void testSet() {
    std::set<int> s1;
    util::Set<int> s2;
    int keys[10000];
    for (int i = 0; i < 10000; i++) {
        srand(i+1);
        int k = rand();
        keys[i] = k;
        s1.insert(k);
        s2.insert(k);
        diffSet(s1, s2);
    }
    for (int i = 0; i < 10000; i++) {
        int k = keys[i];
        s1.erase(k);
        s2.erase(k);
        diffSet(s1, s2);
    }
}

int main(int argc, char *argv[]) {
    memory::Allocater::createLocalKey();
    //httpd::Server svr;
    //svr.start("localhost", "9090");
    //svr.run();
    //memory::Allocater::deleteLocalKey();
    testRBTree();
    testMap();
    
    return 0;
}
