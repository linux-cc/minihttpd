#include "network/socket.h"
#include <errno.h>
#include <vector>

using std::vector;
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
    TcpSocket client;
    if (!client.connect("localhost", argv[1], family)) {
        printf("client connect error: %d:%s\n", errno, strerror(errno));
        return -1;
    }
    char data[1024];
    while (!__quit) {
        printf("client: ");
        char *rp = fgets(data, sizeof(data), stdin);
        if(rp == NULL || strncmp(data, "End", 3) == 0)
        {
            printf("Done\n");
            break;
        }
        client.send(data, strlen(data));
        int n = client.recv(data, sizeof(data));
        if (n <= 0)
            break;
        data[n] = 0;
        printf("server: %s", data);
    }
    client.close();

    return 0;
}

