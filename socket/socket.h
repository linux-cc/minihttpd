#ifndef __SOCKET_SOCKET_H__
#define __SOCKET_SOCKET_H__

#include "socket/addrinfo.h"
#include <netinet/tcp.h>
#include <errno.h>

BEGIN_NS(socket)

class Socket {
public:
    int recv(void *buf, size_t size, int flags = 0);
    int send(const void *buf, size_t size, int flags = 0);
    int send(const void *buf1, size_t size1, const void *buf2, size_t size2);
    int send(const void *buf1, size_t size1, const void *buf2, size_t size2, const void *buf3, size_t size3);
    int recvfrom(void *buf, size_t size, Sockaddr &addr, int flags = 0) {
        return ::recvfrom(_socket, buf, size, flags, addr, &addr.len());
    }
    int sendto(const void *buf, size_t size, const Sockaddr &addr, int flags = 0) {
        return ::sendto(_socket, buf, size, flags, addr, addr.len());
    }
    int setNonblock();
    void close();
    void attach(int socket) {
        _socket = socket;
    }
    operator int() const {
        return _socket;
    }

protected:
    explicit Socket(int socket): _socket(socket) {}
    virtual ~Socket() {}
    bool socket(int family, int socktype, int protocol);
    bool create(const char *host, const char *service, int family, int socktype, int protocol);
    bool connect(const char *host, const char *service, int family, int socktype, int protocol);
    int setOpt(int cmd, int level, int val) {
	    return setsockopt(_socket, level, cmd, &val, (socklen_t)sizeof(val));
    }
    int getOpt(int cmd, int level, int *val) {
	    socklen_t len = (socklen_t)sizeof(int);
	    return getsockopt(_socket, level, cmd, val, &len);
    }

    int _socket;
};

class TcpSocket : public Socket {
public:
    explicit TcpSocket(int socket = -1): Socket(socket) {}
    bool create(const char *host, const char *service, int family = PF_UNSPEC) {
        return Socket::create(host, service, family, SOCK_STREAM, 0);
    }
    bool connect(const char *host, const char *service, int family) {
        return Socket::connect(host, service, family, SOCK_STREAM, 0);
    }

    bool connect(const char *host, const char *service, int ms, int family);
    int accept() {
	    return ::accept(_socket, NULL, NULL);
    }
    bool getpeername(Sockaddr &addr) {
        return ::getpeername(_socket, addr, &addr.len()) == 0; 
    }
    void setNoDelay() {
        setOpt(TCP_NODELAY, IPPROTO_TCP, 1);
    }
};

class UdpSocket : public Socket {
public:
    explicit UdpSocket(int socket = -1): Socket(socket) {}
    bool create(const char *host, const char *service, int family = PF_UNSPEC) {
        return Socket::create(host, service, family, SOCK_DGRAM, 0);
    }
    bool connect(const char *host, const char *service, int family = PF_UNSPEC) {
        return Socket::connect(host, service, family, SOCK_DGRAM, 0);
    }
    using Socket::sendto;
    /* functions used by client */
    int sendto(const char *host, const char *service, const void *buf, size_t size, int family = PF_UNSPEC, int flags = 0);
};

END_NS
#endif /* ifndef __SOCKET_SOCKET_H__ */
