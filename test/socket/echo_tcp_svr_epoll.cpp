#include "socket/tcp_socket.h"
#include "socket/epoll.h"
#include <vector>
#include <unistd.h>

using std::vector;
USING_NS(socket);

static bool __quit = false;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("usage %s port ipversion[0|1|2|3]\n", argv[0]);
        return -1;
    }
    int ip = argv[2][0];
    int family = ip == '0' ? PF_UNSPEC : (ip == '1' ? PF_INET :
            (ip == '2' ? PF_INET6 : PF_LOCAL));
    TcpSocket server;
    EPoller poller;
    if (!server.create("localhost", argv[1], family)) {
        printf("server create error: %d:%s\n", server.errcode(), server.errinfo());
        return -1;
    }
    if (!poller.create(1024)) {
        printf("poller create failed\n");
        return -1;
    }
    printf("listen in port %s\n", argv[1]);
    server.setNonblock();
    poller.add(server);
    char buf[1024];
    while (!__quit) {
        EPollResult result = poller.wait(1000);
        for (EPollResult::Iterator it = result.begin(); it != result.end(); ++it) {
            Sockaddr addr;
            TcpSocket client;
            if (it->fd() == server) {
                while (1) {
                    if (!server.accept(client)) {
                        if (server.errcode() != EWOULDBLOCK && server.errcode() != EAGAIN)
                            printf("server accept error: %d:%s\n", server.errcode(), server.errinfo());
                        poller.mod(server);
                        break;
                    }
                    client.setNonblock();
                    poller.add(client);
                    if (!client.getpeername(addr)) {
                        printf("server getpeername error: %d:%s\n", client.errcode(), client.errinfo());
                    }
                    Peername peer(addr);
                    printf("server accept: [%s|%d]\n", (const char*)peer, peer.port());
                }
            } else {
                client.attach(it->fd());
                if (!client.getpeername(addr)) {
                    printf("server getpeername error: %d:%s\n", client.errcode(), client.errinfo());
                }
                Peername peer(addr);
                int len = client.recv(buf, 1024);
                if (len <= 0) {
                    printf("client[%s|%d]: close, len:%d\n", (const char*)peer, peer.port(), len);
                    poller.del(client);
                    client.close();
                } else {
                    buf[len] = 0;
                    printf("receive data[%s|%d]: %s", (const char*)peer, peer.port(), buf);
                    client.send(buf, len);
                    poller.mod(client);
                }
            }
        }
    }

    return 0;
}

