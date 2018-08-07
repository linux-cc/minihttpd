#ifndef __SOCKET_UDP_SOCKET_H__
#define __SOCKET_UDP_SOCKET_H__

#include "socket/socket.h"

BEGIN_NS(socket)

class UdpSocket : public Socket {
public:
    explicit UdpSocket(int socket = -1): Socket(socket) {}
    bool create(const char *host, const char *service, int family = PF_UNSPEC) {
        return Socket::create(host, service, family, SOCK_DGRAM, 0);
    }
    bool connect(const char *host, const char *service, int family = PF_UNSPEC) {
        return Socket::connect(host, service, family, SOCK_DGRAM, 0);
    }
    using Socket::sendto;
    /* functions used by client */
    int sendto(const char *host, const char *service, const void *buf, size_t size, int family = PF_UNSPEC, int flags = 0);
};

END_NS
#endif /* ifndef __SOCKET_UDP_SOCKET_H__ */
