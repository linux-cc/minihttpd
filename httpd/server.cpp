#include "config.h"
#include "httpd/server.h"
#include "httpd/worker.h"
#include "httpd/connection.h"
#include "httpd/constants.h"

namespace httpd {

using thread::AutoMutex;
using thread::Mutex;

static Mutex __setLock;

Server::Server(int workers, int workerClients, int timeout):
_workers(NULL),
_workerCnt(workers),
_curSlot(0),
_quit(false) {
    Header::init();
    ResponseStatus::init();
    _slotQ.resize(timeout);
    _workers = new Worker*[_workerCnt];
    for (int i = 0; i < _workerCnt; ++i) {
        _workers[i] = new Worker(*this, workerClients);
    }
    _LOG_("workers: %d, workerClients: %d, timeout: %d\n", _workerCnt, workerClients, timeout);
}

Server::~Server() {
    if (_workers) {
        for (int i = 0; i < _workerCnt; ++i) {
            delete _workers[i];
        }
        delete[] _workers;
    }
}

bool Server::start(const char *host, const char *service) {
    if (!_server.create(host, service)) {
        _LOG_("server create error:%d:%s\n", errno, strerror(errno));
        return false;
    }
    _server.setNonblock();
    
    for (int i = 0; i < _workerCnt; ++i) {
        if (!_workers[i]->start()) {
            _LOG_("server start worker error:%d:%s\n", errno, strerror(errno));
            return false;
        }
    }
    _LOG_("server listen on %s:%s\n", host, service);

    return true;
}

void Server::update(Connection *conn, Worker *worker) {
    remove(conn, worker);
    size_t newSlot = _curSlot - 1;
    if (newSlot < 0) {
        newSlot = _slotQ.size() - 1;
    }
    _connSlot[conn] = newSlot;
    SlotSet &newSet = _slotQ.at(newSlot);
    AutoMutex mutex(__setLock);
    newSet.insert(std::make_pair(conn, worker));
}

void Server::remove(Connection *conn, Worker *worker) {
    Item key = std::make_pair(conn, worker);
    SlotMap::iterator it = _connSlot.find(conn);
    if (it != _connSlot.end()) {
        SlotSet &oldSet = _slotQ.at(it->second);
        AutoMutex mutex(__setLock);
        SlotSet::iterator it = oldSet.find(key);
        if (it != oldSet.end()) {
            oldSet.erase(it);
        }
    }
}

void Server::run() {
    while (!_quit) {
        SlotSet &curSet = _slotQ.at(_curSlot);
        {
            AutoMutex mutex(__setLock);
            if (!curSet.empty()) {
                for (set<Item>::iterator it = curSet.begin(); it != curSet.end(); ++it) {
                    _LOG_("Connection timeout, fd: %d, connection: %p\n", (int)*it->first, it->first);
                    it->second->close(it->first);
                }
                curSet.clear();
            }
        }
        if (++_curSlot == _slotQ.size()) {
            _curSlot = 0;
        }
        sleep(1);
    }
}

} /* namespace httpd */

