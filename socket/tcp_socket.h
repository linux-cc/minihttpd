#ifndef __NETWORK_TCP_SOCKET_H__
#define __NETWORK_TCP_SOCKET_H__

#include "network/socket.h"
#include <stdio.h>

BEGIN_NS(network)

class TcpSocket : public Socket {
public:
    bool create(const char *host, const char *service, int family = PF_UNSPEC) {
        return Socket::create(host, service, family, SOCK_STREAM, 0);
    }
    bool connect(const char *host, const char *service, int family) {
        return Socket::connect(host, service, family, SOCK_STREAM, 0);
    }

    bool connect(const char *host, const char *service, int ms, int family);
    bool accept(TcpSocket &client);
    int recvn(void *buf, size_t size, int flags = 0);
    int sendn(const void *buf, size_t size, int flags = 0);
    bool getpeername(Sockaddr &addr) {
        return ::getpeername(_socket, addr, &addr.len()) == 0; 
    }

};

END_NS

#endif /* __NETWORK_TCP_SOCKET_H__ */
