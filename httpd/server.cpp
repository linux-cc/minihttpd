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
        _LOG_("server create error:%d:%s\n", errno, strerror(errno));
        return false;
    }
    _server.setNonblock();
    
    for (int i = 0; i < MAX_WORKER; ++i) {
        if (!_workers[i]->start()) {
            _LOG_("server start worker error:%d:%s\n", errno, strerror(errno));
            return false;
        }
    }
    _LOG_("server listen on %s:%s\n", host, service);

    return true;
}

void Server::update() {
    int newIndex = _curIndex - 1;
    if (newIndex < 0) {
        newIndex = _timeoutQ.size() - 1;
    }
    Item item;
    while (_eventQ.dequeue(item)) {
        if (_connIndex.find(item)) {
            SimpleList<Item> &oldList = _timeoutQ.at(item._queueIndex);
            oldList.erase(item);
        } else {
            item._queueIndex = newIndex;
            _connIndex.push(item);
        }

        SimpleList<Item> &newList = _timeoutQ.at(newIndex);
        newList.push(item);
    }
}

void Server::run() {
    while (!_quit) {
        update();
        SimpleList<Item> &curList = _timeoutQ.at(_curIndex);
        Item data;
        while (curList.pop(data)) {
            _LOG_("Connection timeout, fd: %d, connection: %p\n", data._conn->fd(), data._conn);
            data._conn->close();
        }
        if (++_curIndex == _timeoutQ.capacity()) {
            _curIndex = 0;
        }
        sleep(1);
    }
}

} /* namespace httpd */
