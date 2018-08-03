#ifndef __NETWORK_SOCKET_H__
#define __NETWORK_SOCKET_H__

#include "addrinfo.h"

namespace myframe {
namespace network {

class TcpSocket {
public:
    TcpSocket(): _socket(-1), _errno(0) {}
    void setHints(int family, int socktype, int protocol = 0);
    bool create(const char *host, const char *service);
    bool create(int family, int socktype, int protocol);
    bool connect(const char *host, const char *service);
    bool connect(const char *host, const char *service, int ms);
    bool accept(TcpSocket &client);
    int recv(void *buf, size_t size, int flags = 0);
    int send(const void *buf, size_t size, int flags = 0);
    int recvn(void *buf, size_t size, int flags = 0);
    int sendn(const void *buf, size_t size, int flags = 0);
    int setNonblock();
    void close();
    bool getpeername(char *addr, int addrlen, int *port);

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
        return _errno;
    }
    const char *errinfo() const {
        return strerror(_errno);
    }

private:
    int _socket;
    int _errno;
};

class UdpSocket {
public:
    int recvfrom(void *buf, size_t size, const char *host, const char *service, int flags);
    int sendto(const void *buf, size_t size, const char *host, const char *service, int flags);
    bool getpeername(char *addr, int addrlen, int *port);

private:
    sockaddr _peer;
    socklen_t _len;
};

} /* namespace network */
} /* namespace myframe */

#endif /* ifndef __NETWORK_SOCKET_H__ */
