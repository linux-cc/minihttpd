#ifndef __HTTPD_WORKER_H__
#define __HTTPD_WORKER_H__

#include "config.h"
#include "thread/thread.h"
#include "socket/epoll.h"
#include "httpd/simple_queue.h"

BEGIN_NS(httpd)

USING_CLASS(thread, Thread);
USING_CLASS(socket, EPoller);
USING_CLASS(socket, EPollEvent);
class Server;
class Connection;

class Worker: public Thread {
public:
    Worker(Server &server, int maxClients): _server(server), _maxClients(maxClients) {}
    bool onInit();
    void run();
    void onCancel();
    void close(Connection *conn);
    
private:
    void tryLockAccept(bool &holdLock);
    void disableAccept(bool &holdLock);
    void unlockAccept();
    void onAccept();
    void onHandleEvent();
    void onRequest(EPollEvent &event);
    void onResponse(EPollEvent &event);
    void closeInternal(Connection *conn);

    Server &_server;
    EPoller _poller; 
    SimpleQueue<Connection> _connsQ;
    SimpleQueue<EPollEvent> _eventQ;
    Connection *_conns;
    int _maxClients;
};

END_NS
#endif /* ifndef __HTTPD_WORKER_H__ */
