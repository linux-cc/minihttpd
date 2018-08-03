#ifndef __NETWORK_TCP_SOCKET_H__
#define __NETWORK_TCP_SOCKET_H__

#include "network/socket.h"

BEGIN_NS(network)

class TcpSocket : public Socket {
public:
    bool create(const char *host, const char *service);
    bool connect(const char *host, const char *service);
    bool connect(const char *host, const char *service, int ms);
    bool accept(TcpSocket &client);
    int recvn(void *buf, size_t size, int flags = 0);
    int sendn(const void *buf, size_t size, int flags = 0);
    bool getpeername(char *addr, int addrLen, int *port);

};

END_NS

#endif /* __NETWORK_TCP_SOCKET_H__ */
