#include "network/socket.h"
#include <errno.h>
#include <stdio.h>

using namespace network;

static bool __quit = false;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage %s port ipversion[0|1|2|3]\n", argv[0]);
        return -1;
    }
    TcpSocket::Family family;
    switch(argv[2][0]) {
    case '1': family = TcpSocket::F_INET4; break;
    case '2': family = TcpSocket::F_INET6; break;
    case '3': family = TcpSocket::F_LOCAL; break;
    default: family = TcpSocket::F_UNSPEC;
    }
    UdpSocket server;
    if (!server.create("localhost", argv[1], family)) {
        printf("server create error: %d:%s\n", errno, strerror(errno));
        return -1;
    }
    printf("linsten in port %s\n", argv[1]);
    char buf[1024];
    while (!__quit) {
        Sockaddr addr;
        int n = server.recvfrom(buf, sizeof(buf), addr);
        if (n <= 0) {
            printf("server recvfrom error: %d:%s\n", errno, strerror(errno));
            continue;
        }
        buf[n] = 0;
        Peername peer(addr);
        printf("receive data[%s|%d]: %s", peer.name(), peer.port(), buf);
        if (server.sendto(buf, n, addr) <= 0) {
            printf("server sendto error: %d:%s\n", errno, strerror(errno));
        }
    }

    return 0;
}

