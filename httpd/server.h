#ifndef __HTTPD_SERVER_H__
#define __HTTPD_SERVER_H__

#include "config.h"
#include "socket/socket.h"
#include "httpd/simple_queue.h"
#include <set>
#include <map>

BEGIN_NS(httpd)

USING_CLASS(socket, TcpSocket);
using std::set;
using std::map;
class Worker;
class Connection;

class Server {
public:
    Server();
    bool start(int workers, int workerClients, int timeout);
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
    int _slots;
    int _curSlot;
    bool _quit;
};

END_NS
#endif /* ifndef __HTTPD_SERVER_H__ */
