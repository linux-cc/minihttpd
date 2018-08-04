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
    UdpSocket client;
    if (family == PF_LOCAL && !client.create("client", "9000", family)) {
        printf("client create error: %d:%s\n", client.errcode(), client.errinfo());
        return -1;
    }
    Sockaddr addr;
    char data[1024];
    while (!__quit) {
        printf("client: ");
        char *rp = fgets(data, sizeof(data), stdin);
        if(rp == NULL || strncmp(data, "End", 3) == 0)
        {
            printf("Done\n");
            break;
        }
        client.sendto("localhost", argv[1], data, strlen(data), family);
        int n = client.recvfrom(data, sizeof(data), addr);
        if (n <= 0)
            break;
        data[n] = 0;
        printf("server: %s", data);
    }
    client.close();

    return 0;
}

