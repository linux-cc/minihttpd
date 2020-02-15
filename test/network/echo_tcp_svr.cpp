#include "config.h"
#include "network/socket.h"
#include <vector>
#include <unistd.h>

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
    TcpSocket server;
    if (!server.create("localhost", argv[1], family)) {
        printf("server connect error: %d:%s\n", errno, strerror(errno));
        return -1;
    }
    printf("listen in port %s\n", argv[1]);
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
    while (!__quit) {
        fd_set rfds = fds;
        int n = select(maxFd + 1, &rfds, NULL, NULL, &tv);
        for (int i = 0; n > 0 && i < (int)vfds.size(); ++i) {
            if (n == 0 || !FD_ISSET(vfds[i], &rfds))
                continue;
            TcpSocket &sock = vfds[i];
            if (sock == server) {
                int fd = server.accept();
                if (fd < 0) {
                    printf("server accept error: %d:%s\n", errno, strerror(errno));
                    continue;
                }
                TcpSocket client(fd);
                client.setNonblock();
                vfds.push_back(client);
                FD_SET(client, &fds);
                if (client > maxFd) 
                    maxFd = client;
                Peername peer = client.getPeerName();
                printf("server accept: [%s|%d]\n", peer.name(), peer.port());
            } else {
                Peername peer = sock.getPeerName();
                int len = sock.recv(buf, 1024);
                if (len <= 0) {
                    printf("client[%s|%d]: close, len:%d\n", peer.name(), peer.port(), len);
                    FD_CLR(sock, &fds);
                    sock.close();
                    vfds.erase(vfds.begin() + i);
                } else {
                    buf[len] = 0;
                    printf("receive data[%s|%d]: %s", peer.name(), peer.port(), buf);
                    sock.send(buf, len);
                }
            }
            --n;
        }
    }

    return 0;
}

