#ifndef __SOCKET_SOCKET_H__
#define __SOCKET_SOCKET_H__

#include "network/addrinfo.h"
#include <netinet/tcp.h>

struct iovec;
namespace network {

class Socket {
public:
    enum Family {
        F_UNSPEC = PF_UNSPEC,
        F_INET4 = PF_INET,
        F_INET6 = PF_INET6,
        F_LOCAL = PF_LOCAL,
    };
public:
    ssize_t recv(void *buf, size_t size);
    ssize_t recv(void *buf1, size_t size1, void *buf2, size_t size2);
    ssize_t recv(struct iovec *iov, int iovcnt);
    ssize_t send(const void *buf, size_t size);
    ssize_t send(const void *buf1, size_t size1, const void *buf2, size_t size2);
    ssize_t send(struct iovec *iov, int iovcnt);

    ssize_t recvfrom(void *buf, size_t size, Sockaddr &addr, int flags = 0) {
        return ::recvfrom(_socket, buf, size, flags, addr, &addr.len());
    }
    ssize_t sendto(const void *buf, size_t size, const Sockaddr &addr, int flags = 0) {
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
    bool isClosed() const {
        return _socket == -1;
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
    bool create(const char *host, const char *service, Family family = F_UNSPEC) {
        return Socket::create(host, service, family, SOCK_STREAM, 0);
    }
    bool connect(const char *host, const char *service, Family family) {
        return Socket::connect(host, service, family, SOCK_STREAM, 0);
    }

    bool connect(const char *host, const char *service, int ms, Family family);
    int accept() {
	    return ::accept(_socket, NULL, NULL);
    }
    Peername getPeerName() {
        Sockaddr addr;
        if (getpeername(_socket, addr, &addr.len()) == 0) {
            return Peername(addr);
        }
        return Peername();
    }
    void setNoDelay() {
        setOpt(TCP_NODELAY, IPPROTO_TCP, 1);
    }
};

class UdpSocket : public Socket {
public:
    explicit UdpSocket(int socket = -1): Socket(socket) {}
    bool create(const char *host, const char *service, Family family = F_UNSPEC) {
        return Socket::create(host, service, family, SOCK_DGRAM, 0);
    }
    bool connect(const char *host, const char *service, Family family = F_UNSPEC) {
        return Socket::connect(host, service, family, SOCK_DGRAM, 0);
    }
    using Socket::sendto;
    /* functions used by client */
    ssize_t sendto(const char *host, const char *service, const void *buf, size_t size, Family family = F_UNSPEC, int flags = 0);
};

} /* namespace network */
#endif /* ifndef __SOCKET_SOCKET_H__ */
