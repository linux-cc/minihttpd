#ifndef __HTTPD_TCP_READER_H__
#define __HTTPD_TCP_READER_H__

#include "config.h"
#include "thread/thread.h"
#include "socket/epoll.h"
#include "socket/connection.h"
#include "socket/tcp_socket.h"

USING_CLASS(thread, Thread);
USING_CLASS(socket, EPoller);
USING_CLASS(socket, Connection);
USING_CLASS(socket, TcpSocket);

BEGIN_NS(httpd)

class TcpReader : public Thread {
public:
    TcpReader(int maxClients = 1024): _free(NULL), _nodes(NULL), _maxClients(maxClients), _quit(false) {}
    bool onInit();
    void run();
    void onCancel();
    
    void quit() {
        _quit = true;
    }

private:
    void onAccept();

    struct Node {
        Connection conn;
        Node *next;
    };
    TcpSocket _server;
    EPoller _poller; 
    Node *_free;
    Node *_nodes;
    int _maxClients;
    bool _quit;
};

END_NS
#endif /* ifndef __HTTPD_TCP_SERVER_H__ */
