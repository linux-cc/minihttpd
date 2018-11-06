#ifndef __FCGID_H__
#define __FCGID_H__

#include "socket/socket.h"

BEGIN_NS(fcgid)

USING_CLASS(socket, TcpSocket);

class Manager {
public:
    Manager(int children = 4, bool isLocal = true);
    bool start(const char *host, const char *service);

private:
    void process();
    void monitor();

    int _children;
    bool _local;
    TcpSocket _socket;
};

END_NS
#endif /* ifndef __FCGID_H__ */
