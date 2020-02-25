#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "util/config.h"
#include "util/simple_list.h"
#include "util/simple_queue.h"
#include "network/socket.h"

namespace httpd {

using network::TcpSocket;
using namespace util;
class Worker;
class Connection;

class Server {
public:
    Server();
    ~Server();
    bool start(const char *host, const char *service);
    void run();

    int accept() { return _server.accept(); }
    operator int () { return _server; }
    void quit() { _quit = true; }
    bool isQuit() { return _quit; }
    
private:
    void update();
    
    struct Item {
        Item(Connection *conn = NULL): _conn(conn) {}
        Connection *_conn;
        int _queueIndex;
        bool operator==(const Item &other) { return _conn == other._conn; }
    };
    
    TcpSocket _server;
    SimpleQueue<SimpleList<Item> > _timeoutQ;
    BlockQueue<Item> _eventQ;
    SimpleList<Item> _connIndex;
    Worker *_workers[MAX_WORKER];
    int _curIndex;
    bool _quit;
    
    friend class Worker;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_SERVER_H__ */
