#include "socket.h"
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>

BEGIN_NS(network)

static int wait(int socket, int ms);

bool TcpSocket::create(const char *host, const char *service) {
    Addrinfo ai(PF_UNSPEC, SOCK_STREAM, 0);
    if ((_errno = ai.getAddrinfo(host, service))) {
        return false;
    }
    close();
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if ((_socket = socket(it->ai_family, it->ai_socktype, it->ai_protocol)) < 0) {
            continue;
        }
        setOpt(SO_REUSEADDR, 1);
        if (bind(_socket, it->ai_addr, it->ai_addrlen) < 0 || listen(_socket, 5) < 0) {
            close();
            continue;
        }
        return true;
    }
    _errno = errno;
	return false;
}

bool TcpSocket::connect(const char *host, const char *service) {
    Addrinfo ai(PF_UNSPEC, SOCK_STREAM, 0);
    if ((_errno = ai.getAddrinfo(host, service))) {
        return false;
    }
    close();
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if ((_socket = socket(it->ai_family, it->ai_socktype, it->ai_protocol)) < 0) {
            continue;
        }
	    if(::connect(_socket, it->ai_addr, it->ai_addrlen) < 0) {
            close();
	    	continue;
        }
	    return true;
    }
    _errno = errno;
    return false;
}

bool TcpSocket::connect(const char *host, const char *service, int ms) {
	int oldFlags = setNonblock();
	if(!connect(host, service)) {
        if((errno != EINPROGRESS && errno != EWOULDBLOCK) || wait(_socket, ms) <= 0) {
            _errno = errno;
			return false;
        }
	    int error = 0;
        if(getOpt(SO_ERROR, &error) < 0 || error) {
            _errno = errno;
			return false;
        }
	}
	fcntl(_socket, F_SETFL, oldFlags);

	return true;
}

bool TcpSocket::accept(TcpSocket &client) {
	int fd = ::accept(_socket, NULL, NULL);
	if(fd < 0) {
        _errno = errno;
		return false;
    }
	client.attach(fd);
	
    return true;
}

int TcpSocket::recv(void *buf, size_t size, int flags) {
    int n;
    while (true){
	    n = ::recv(_socket, buf, size, flags);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }
    _errno = errno;
	return n;
}

int TcpSocket::send(const void *buf, size_t size, int flags) {
    int n;
    while (true){
	    n = ::send(_socket, buf, size, flags);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }
    _errno = errno;
	return n;
}

int TcpSocket::recvn(void *buf, size_t size, int flags) {
    size_t left = size;
    char *p = (char*)buf;

    while (left > 0) {
        int nr = recv(p, left, flags);
        if (nr <= 0) {
            return nr;
        }
        left -= nr;
        p += nr;
    }
    return size - left;
}

int TcpSocket::sendn(const void *buf, size_t size, int flags) {
    size_t left = size; 
    const char *p = (const char*)buf;

    while (left > 0) {
        int ns = send(p, left, flags);
        if (ns < 0) {
            return ns;
        }
        left -= ns;
        p += ns;
    }
    return size - left;
}

int TcpSocket::setNonblock() {
	int flags = fcntl(_socket, F_GETFL);
	fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
    return flags;
}

void TcpSocket::close() {
	if(_socket != -1) {
		::close(_socket);
		_socket = -1;
	}
}

bool TcpSocket::getpeername(char *addr, int addrlen, int *port) {
    sockaddr_storage peer;
    socklen_t len = sizeof(peer);
    if (::getpeername(_socket, (sockaddr*)&peer, &len) < 0) {
        _errno = errno;
        return false;
    }
    void *inaddr;
    if (peer.ss_family == PF_INET) {
        inaddr = &((sockaddr_in*)&peer)->sin_addr;
        *port = ntohs(((sockaddr_in*)&peer)->sin_port);
    } else if (peer.ss_family == PF_INET6) {
        inaddr = &((sockaddr_in6*)&peer)->sin6_addr;
        *port = ntohs(((sockaddr_in6*)&peer)->sin6_port);
    } else {
        return false;
    }
    if (!inet_ntop(peer.ss_family, inaddr, addr, addrlen)) {
        _errno = errno;
        return false;
    }
    return true;
}

int wait(int socket, int ms)
{
	fd_set rfds;
	struct timeval tv;

	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms % 1000) * 1000;
	FD_ZERO(&rfds);
	FD_SET(socket, &rfds);

	int result = select(socket + 1, &rfds, NULL, NULL, &tv);

	if (FD_ISSET(socket, &rfds))
		return result;

	return 0;
}

int UdpSocket::recvfrom(void *buf, size_t size, const char *host, const char *service, int flags) {
    Addrinfo ai(PF_UNSPEC, SOCK_DGRAM, 0);
    if (ai.getAddrinfo(host, service)) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        int sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock < 0) {
            continue;
        }
        return ::recvfrom(sock, buf, size, flags, &_peer, &_len);
    }
    return -1;
}

int UdpSocket::sendto(const void *buf, size_t size, const char *host, const char *service, int flags) {
    Addrinfo ai(PF_UNSPEC, SOCK_DGRAM, 0);
    if (ai.getAddrinfo(host, service)) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        int sock = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
        if (sock < 0) {
            continue;
        }
        return ::sendto(sock, buf, size, flags, it->ai_addr, it->ai_addrlen);
    }

    return -1;
}

bool UdpSocket::getpeername(char *addr, int addrlen, int *port) {
    if (!inet_ntop(_peer.sa_family, &_peer, addr, addrlen)) {
        return false;
    }
    *port = ntohs(*(uint16_t*)_peer.sa_data);
    return true;
}

END_NS
#ifdef __SOCKET_SERVER_TCP__

#include <vector>
using std::vector;
using namespace myframe::network;
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

#endif

#ifdef __SOCKET_CLIENT_TCP__

#include <stdio.h>
using namespace myframe::network;
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
        printf("server: %s\n", data);
    }
    client.close();

    return 0;
}

#endif
