#include "network/udp_socket.h"

BEGIN_NS(network)

int UdpSocket::sendto(const char *host, const char *service, const void *buf, size_t size, int family, int flags) {
    Addrinfo ai(family, SOCK_DGRAM, 0);
    if (ai.getaddrinfo(host, service)) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (!socket(it->ai_family, it->ai_socktype, it->ai_protocol)) {
            continue;
        }
        return Socket::sendto(buf, size, Sockaddr(it->ai_addr, it->ai_addrlen), flags);
    }

    return -1;
}

END_NS
