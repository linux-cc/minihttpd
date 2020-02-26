#ifndef __HTTPD_WORKER_H__
#define __HTTPD_WORKER_H__

#include "util/simple_list.h"
#include "thread/thread.h"
#include "network/epoll.h"

namespace httpd {

using namespace util;
using thread::Thread;
using network::EPoller;
using network::EPollEvent;
class Server;
class Connection;

class Worker: public Thread {
public:
    Worker(Server &server): _server(server), _actives(0), _acceptLock(false) {}
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

    Server &_server;
    EPoller _poller; 
    SimpleList<Connection*> _connsList;
    SimpleList<EPollEvent*> _eventList;
    int _actives;
    bool _acceptLock;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_WORKER_H__ */
