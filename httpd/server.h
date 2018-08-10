#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "config.h"
#include "thread/thread.h"
#include "socket/socket.h"
#include "socket/epoll.h"
#include "httpd/connection.h"
#include "utils/cycle_queue.h"

BEGIN_NS(httpd)

USING_CLASS(thread, Thread);
USING_CLASS(socket, EPoller);
USING_CLASS(socket, EPollEvent);
USING_CLASS(socket, TcpSocket);
USING_CLASS(utils, CycleQueue);
class Request;
class Worker;

class Server {
public:
    Server(): _workers(NULL), _quit(false) {}
    bool start(int workers, int maxClients = 1024);
    int accept() {
        return _server.accept();
    }
    operator int () {
        return _server;
    }
    void quit() {
        _quit = true;
    }
    bool isQuit() {
        return _quit;
    }
private:
    TcpSocket _server;
    Worker **_workers;
    bool _quit;
};

class Worker: public Thread {
public:
    Worker(Server &server, int maxClients): _server(server), _maxClients(maxClients) {}
    bool onInit();
    void run();
    void onCancel();
    
private:
    void tryLockAccept(bool &holdLock);
    void unlockAccept();
    void onAccept();
    void onHandleEvent();
    void onRequest(EPollEvent &event);
    bool readHeader(Connection *conn, Request &request);
    bool readContent(Connection *conn, Request &request);
    void onResponse(Connection *conn, Request &request);
    void close(Connection *conn);

    Server &_server;
    EPoller _poller; 
    CycleQueue<Connection> _connsQ;
    CycleQueue<EPollEvent> _eventQ;
    Connection *_conns;
    int _maxClients;
};

END_NS
#endif /* ifndef __HTTPD_SERVER_H__ */
