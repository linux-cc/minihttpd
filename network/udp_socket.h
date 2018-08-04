#ifndef __NETWORK_UDP_SOCKET_H__
#define __NETWORK_UDP_SOCKET_H__

#include "network/socket.h"

BEGIN_NS(network)

class UdpSocket : public Socket {
public:
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
#endif
