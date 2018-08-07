#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "config.h"
#include "socket/tcp_socket.h"
#include "thread/thread.h"
#include "socket/epoll.h"
#include "socket/connection.h"

BEGIN_NS(httpd)

USING_CLASS(thread, Thread);
USING_CLASS(socket, EPoller);
USING_CLASS(socket, EPollEvent);
USING_CLASS(socket, Connection);
USING_CLASS(socket, TcpSocket);

class Server {
public:
    bool onInit();

    bool accept(TcpSocket &client) {
        return _server.accept(client);
    }
    operator int () {
        return _server;
    }
    int errcode() {
        return _server.errcode();
    }
    const char *errinfo() {
        return _server.errinfo();
    }
private:
    TcpSocket _server;
};

class Worker: public Thread {
public:
    Worker(Server &server, int maxClients = 1024);
    bool onInit();
    void run();
    void onCancel();
    
    void quit() {
        _quit = true;
    }

private:
    void enableAccept();
    void disableAccept();
    void onAccept();
    void onPollIn(EPollEvent &event);

    struct Node {
        Connection conn;
        Node *next;
    };
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
