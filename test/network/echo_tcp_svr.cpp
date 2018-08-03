#include "network/tcp_socket.h"
#include <sys/select.h>
#include <vector>

using std::vector;
USING_NS(network);

static bool __quit = false;

int main(int argc, char *argv[]) {
    TcpSocket server;
    if (!server.create("localhost", argv[1])) {
        printf("server create error: %d:%s\n", server.errcode(), server.errinfo());
        return -1;
    }
    printf("linsten in port %s\n", argv[1]);
    vector<TcpSocket> vfds;
    fd_set fds;
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(server, &fds);
    int maxFd = server;
    vfds.push_back(server);
    server.setNonblock();
    char buf[1024];
    char addr[64];
    int port;
    while (!__quit) {
        fd_set rfds = fds;
        int n = select(maxFd + 1, &rfds, NULL, NULL, &tv);
        for (int i = 0; i < vfds.size(); ++i) {
            if (!FD_ISSET(vfds[i], &rfds))
                continue;
            TcpSocket &sock = vfds[i];
            if (sock == server) {
                TcpSocket client;
                if (!server.accept(client)) {
                    printf("server accept error: %d:%s\n", server.errcode(), server.errinfo());
                    continue;
                }
                client.setNonblock();
                vfds.push_back(client);
                FD_SET(client, &fds);
                if (client > maxFd) 
                    maxFd = client;
                if (!client.getpeername(addr, sizeof(addr), &port)) {
                    printf("server getpeername error: %d:%s\n", client.errcode(), client.errinfo());
                }
                printf("server accept: [%s|%d]\n", addr, port);
            } else {
                if (!sock.getpeername(addr, sizeof(addr), &port)) {
                    printf("server getpeername error: %d:%s\n", sock.errcode(), sock.errinfo());
                }
                int n = sock.recv(buf, 1024);
                if (n <= 0) {
                    printf("client[%s|%d]: close\n", addr, port);
                    vfds.erase(vfds.begin() + i);
                } else {
                    buf[n] = 0;
                    printf("receive data[%s|%d]: %s", addr, port, buf);
                    sock.send(buf, n);
                }
            }
        }
    }

    return 0;
}

