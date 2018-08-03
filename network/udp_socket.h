#ifndef __NETWORK_UDP_SOCKET_H__
#define __NETWORK_UDP_SOCKET_H__

#include "network/socket.h"

BEGIN_NS(network)

class UdpSocket : public Socket {
public:
    UdpSocket(const char *host, const char *service, int family, bool local):
        Socket(family, local), _host(host), _service(service) {}
    int recvfrom(void *buf, size_t size, char *addr, int addrlen, int *port, int flags = 0);
    int sendto(const void *buf, size_t size, int flags = 0);

private:
    const char *_host;
    const char *_service;
};

END_NS
#endif
