#include "network/tcp_socket.h"
#include <vector>

using std::vector;
USING_NS(network);

static bool __quit = false;

int main(int argc, char *argv[]) {
    TcpSocket client;
    if (!client.connect("localhost", argv[1])) {
        printf("client connect error: %d:%s\n", client.errcode(), client.errinfo());
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

