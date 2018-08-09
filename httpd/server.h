#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "config.h"
#include "thread/thread.h"
#include "socket/socket.h"
#include "socket/epoll.h"
#include "httpd/connection.h"

BEGIN_NS(httpd)

USING_CLASS(thread, Thread);
USING_CLASS(socket, EPoller);
USING_CLASS(socket, EPollEvent);
USING_CLASS(socket, TcpSocket);

class Server {
public:
    bool onInit();

    int accept() {
        return _server.accept();
    }
    operator int () {
        return _server;
    }
private:
    TcpSocket _server;
};

class Worker: public Thread {
public:
    Worker(Server &server, int maxClients = 256);
    bool onInit();
    void run();
    void onCancel();
    
    void quit() {
        _quit = true;
    }

private:
    struct Node {
        Connection conn;
        Node *next;
    };
    void enableAccept();
    void disableAccept();
    void onAccept();
    void onRequest(EPollEvent &event);
    void close(Node *node);

    Server &_server;
    EPoller _poller; 
    Node *_free;
    Node *_nodes;
    int _maxClients;
    bool _quit;
    bool _holdLock;
};

END_NS
#endif /* ifndef __HTTPD_SERVER_H__ */
