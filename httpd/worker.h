#ifndef __HTTPD_WORKER_H__
#define __HTTPD_WORKER_H__

#include "util/simple_list.h"
#include "thread/thread.h"
#include "network/epoll.h"
#include "memory/simple_alloc.h"

namespace httpd {

using util::SimpleList;
using thread::Thread;
using network::EPoller;
using network::EPollEvent;
using memory::Allocater;
class Server;
class Connection;
class Response;
class Worker: public Thread {
public:
    Worker(Server &server): _server(server), _alloc(8192, 512), _actives(0), _acceptLock(false) {}
    bool onInit();
    void run();
    void close(Connection *conn);
    
private:
    bool tryLockAccept();
    bool enableAccept();
    void disableAccept();
    void unlockAccept();
    void onAccept();
    void onHandleEvent();
    void onRequest(EPollEvent &event);
    void onResponse(EPollEvent &event);
    void onResponse(Response *resp, Connection *conn);

    Server &_server;
    Allocater _alloc;
    EPoller _poller; 
    SimpleList<Connection*> _connsList;
    SimpleList<EPollEvent*> _eventList;
    int _actives;
    bool _acceptLock;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_WORKER_H__ */
