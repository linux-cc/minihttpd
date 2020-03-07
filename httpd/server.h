#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "util/config.h"
#include "util/simple_list.h"
#include "util/simple_queue.h"
#include "network/socket.h"

namespace httpd {

using util::SimpleQueue;
using util::BlockQueue;
using util::SimpleList;
using network::TcpSocket;
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
    void update(Connection *conn, bool closed = false);
    
private:
    void update();
    
    struct Item {
        Item(Connection *conn = NULL, bool closed = false);
        int _fd;
        int _oldIdx : 15;
        int _newIdx : 15;
        int _closed : 2;
        bool operator==(const Item &r) {
            return _fd == r._fd && _oldIdx == r._oldIdx && _newIdx == r._newIdx;
        }
    };
    TcpSocket _server;
    SimpleQueue<SimpleList<Item> > _timeoutQ;
    BlockQueue<Item> _eventQ;
    Worker *_workers[MAX_WORKER];
    int _curIndex;
    bool _quit;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_SERVER_H__ */
