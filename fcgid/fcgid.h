#ifndef __FCGID_H__
#define __FCGID_H__

#include "config.h"
#include "network/socket.h"

namespace fcgid {

class Manager {
public:
    Manager(int children = 4, bool isLocal = true);
    bool start(const char *host, const char *service);

private:
    void process();
    void monitor();

    int _children;
    bool _local;
    network::TcpSocket _socket;
};

}
#endif /* ifndef __FCGID_H__ */
