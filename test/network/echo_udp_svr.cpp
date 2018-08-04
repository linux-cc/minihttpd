#include "network/udp_socket.h"
#include <stdio.h>

USING_NS(network);

static bool __quit = false;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage %s port ipversion[0|1|2|3]\n", argv[0]);
        return -1;
    }
    int ip = argv[2][0];
    int family = ip == '0' ? PF_UNSPEC : (ip == '1' ? PF_INET :
            (ip == '2' ? PF_INET6 : PF_LOCAL));
    UdpSocket server;
    if (!server.create("localhost", argv[1], family)) {
        printf("server create error: %d:%s\n", server.errcode(), server.errinfo());
        return -1;
    }
    printf("linsten in port %s\n", argv[1]);
    char buf[1024];
    while (!__quit) {
        Sockaddr addr;
        int n = server.recvfrom(buf, sizeof(buf), addr);
        if (n <= 0) {
            printf("recvfrom error: %d:%s\n", server.errcode(), server.errinfo());
            continue;
        }
        buf[n] = 0;
        Peername peer(addr);
        printf("receive data[%s|%d]: %s", (const char*)peer, peer.port(), buf);
        if (server.sendto(buf, n, addr) <= 0) {
            printf("sendto error: %d:%s\n", server.errcode(), server.errinfo());
        }
    }

    return 0;
}

