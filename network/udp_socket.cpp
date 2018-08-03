#include "network/udp_socket.h"

BEGIN_NS(network)

int UdpSocket::recvfrom(void *buf, size_t size, char *addr, int addrLen, int *port, int flags) {
    Addrinfo ai(PF_UNSPEC, SOCK_DGRAM, 0);
    if ((_errno = ai.getaddrinfo(_host, _service))) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (!socket(it->ai_family, it->ai_socktype, it->ai_protocol)) {
            continue;
        }
        return Socket::recvfrom(buf, size, addr, addrLen, port, flags);
    }

    return -1;
}

int UdpSocket::sendto(const void *buf, size_t size, int flags) {
    Addrinfo ai(PF_UNSPEC, SOCK_DGRAM, 0);
    if (ai.getaddrinfo(_host, _service)) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (!socket(it->ai_family, it->ai_socktype, it->ai_protocol)) {
            continue;
        }
        return Socket::sendto(buf, size, it->ai_addr, it->ai_addrlen, flags);
    }

    return -1;
}

END_NS
