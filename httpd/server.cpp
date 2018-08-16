#include "httpd/server.h"
#include "httpd/worker.h"

BEGIN_NS(httpd)

USING_CLASS(thread, AutoMutex);
USING_CLASS(thread, Mutex);

static Mutex __setLock;

bool Server::start(int workers, int workerClients, int timeout) {
    _LOG_("workers: %d, workerClients: %d, timeout: %d\n", workers, workerClients, timeout);
    if (!_server.create(NULL, "9090")) {
        _LOG_("server create error:%d:%s\n", errno, strerror(errno));
        return false;
    }
    _server.setNonblock();
    _workers = new Worker*[workers];
    for (int i = 0; i < workers; ++i) {
        _workers[i] = new Worker(*this, workerClients);
        _workers[i]->start();
    }
    _LOG_("server listen on port 9090\n");
    _slots = timeout;
    _slotSets = new SlotSet[timeout];    
    _slotQ.init(_slotSets, timeout);

    return true;
}

void Server::update(Connection *conn, Worker *worker) {
    remove(conn, worker);
    int newSlot = _curSlot - 1;
    if (newSlot < 0) {
        newSlot = _slots - 1;
    }
    _connSlot[conn] = newSlot;
    SlotSet *newSet = _slotQ.at(newSlot);
    AutoMutex mutex(__setLock);
    newSet->insert(std::make_pair(conn, worker));
}

void Server::remove(Connection *conn, Worker *worker) {
    Item key = std::make_pair(conn, worker);
    SlotMap::iterator it = _connSlot.find(conn);
    if (it != _connSlot.end()) {
        SlotSet *oldSet = _slotQ.at(it->second);
        AutoMutex mutex(__setLock);
        SlotSet::iterator it = oldSet->find(key);
        if (it != oldSet->end()) {
            oldSet->erase(it);
        }
    }
}

void Server::run() {
    while (!_quit) {
        SlotSet *curSet = _slotQ.at(_curSlot);
        {
            AutoMutex mutex(__setLock);
            if (!curSet->empty()) {
                for (set<Item>::iterator it = curSet->begin(); it != curSet->end(); ++it) {
                    _LOG_("Connection timeout, fd: %d, connection: %p\n", (int)*it->first, it->first);
                    it->second->close(it->first);
                }
                curSet->clear();
            }
        }
        if (++_curSlot == _slots) {
            _curSlot = 0;
        }
        sleep(1);
    }
}

END_NS

