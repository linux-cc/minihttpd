#ifndef __HTTPD_WORKER_H__
#define __HTTPD_WORKER_H__

#include "thread/thread.h"
#include "network/epoll.h"
#include <list>

namespace httpd {

using thread::Thread;
using network::EPoller;
using network::EPollEvent;
using std::list;
class Server;
class Connection;

class Worker: public Thread {
public:
    Worker(Server &server, int maxClients): _server(server), _maxClients(maxClients), _actives(0), _acceptLock(false) {}
    bool onInit();
    void run();
    void onCancel();
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
    void closeInternal(Connection *conn);

    Server &_server;
    EPoller _poller; 
    list<Connection*> _connsQ;
    list<EPollEvent*> _eventQ;
    int _maxClients;
    int _actives;
    bool _acceptLock;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_WORKER_H__ */
