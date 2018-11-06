#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "network/socket.h"
#include "httpd/simple_queue.h"
#include <set>
#include <map>

namespace httpd {

using network::TcpSocket;
using std::set;
using std::map;
class Worker;
class Connection;

class Server {
public:
    Server(int workers = 4, int workerClients = 8, int timeout = 30);
    ~Server();
    bool start(const char *host, const char *service);
    void update(Connection *conn, Worker *worker);
    void remove(Connection *conn, Worker *worker);
    void run();

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
    typedef std::pair<Connection*, Worker*> Item;
    typedef set<Item> SlotSet;
    typedef map<Connection*, int> SlotMap;
    SimpleQueue<SlotSet> _slotQ;
    SlotMap  _connSlot;
    Worker **_workers;
    SlotSet *_slotSets;
    int _workerCnt;
    int _workerClients;
    int _slots;
    int _curSlot;
    bool _quit;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_SERVER_H__ */
