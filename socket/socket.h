#ifndef __NETWORK_SOCKET_H__
#define __NETWORK_SOCKET_H__

#include "socket/addrinfo.h"
#include <errno.h>
#include <stdio.h>

BEGIN_NS(socket)

class Socket {
public:
    int recv(void *buf, size_t size, int flags = 0);
    int send(const void *buf, size_t size, int flags = 0);
    int recvfrom(void *buf, size_t size, Sockaddr &addr, int flags = 0) {
        return ::recvfrom(_socket, buf, size, flags, addr, &addr.len());
    }
    int sendto(const void *buf, size_t size, const Sockaddr &addr, int flags = 0) {
        return ::sendto(_socket, buf, size, flags, addr, addr.len());
    }
    int setNonblock();
    void close();

    int setOpt(int cmd, int val) {
	    return setsockopt(_socket, SOL_SOCKET, cmd, &val, (socklen_t)sizeof(val));
    }
    int getOpt(int cmd, int *val) {
	    socklen_t len = (socklen_t)sizeof(int);
	    return getsockopt(_socket, SOL_SOCKET, cmd, val, &len);
    }
    void attach(int socket) {
        _socket = socket;
    }
    operator int() const {
        return _socket;
    }
    int errcode() const {
        return errno;
    }
    const char *errinfo() const {
        return strerror(errno);
    }

protected:
    Socket(): _socket(-1) {}
    virtual ~Socket() {}
    bool socket(int family, int socktype, int protocol);
    bool create(const char *host, const char *service, int family, int socktype, int protocol);
    bool connect(const char *host, const char *service, int family, int socktype, int protocol);

    int _socket;
};

END_NS
#endif /* ifndef __NETWORK_SOCKET_H__ */
