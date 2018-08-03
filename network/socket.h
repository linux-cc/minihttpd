#ifndef __NETWORK_SOCKET_H__
#define __NETWORK_SOCKET_H__

#include "config.h"
#include <netdb.h>

BEGIN_NS(network)

class Socket {
public:
    int recv(void *buf, size_t size, int flags = 0);
    int send(const void *buf, size_t size, int flags = 0);
    int recvfrom(void *buf, size_t size, char *addr, int addrLen, int *port, int flags = 0);
    int sendto(const void *buf, size_t size, const sockaddr *addr, socklen_t addrLen, int flags = 0);
    int setNonblock();
    bool getnameinfo(const sockaddr_storage *srcAddr, char *dstAddr, int detLen, int *port);
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
        return _errno;
    }
    const char *errinfo() const {
        return strerror(_errno);
    }

protected:
    Socket(): _socket(-1), _errno(0) {}
    virtual ~Socket() {}
    bool socket(int family, int socktype, int protocol);
    bool bind(sockaddr *addr, socklen_t len);
    bool listen(int backlog = 8);
    bool connect(sockaddr *addr, socklen_t len);

    int _socket;
    int _errno;
};

class Addrinfo {
public:
    class Iterator {
    public:
        Iterator &operator++() {
            _cur = _cur->ai_next;
            return *this;
        }
        const Iterator operator++(int) {
            Iterator old = *this;
            ++(*this);
            return old;
        }
        bool operator!=(const Iterator &other) {
            return _cur != other._cur;
        }
        const addrinfo *operator->() {
            return _cur;
        }

    private:
        Iterator(const addrinfo *result): _cur(result) {}
        
        const addrinfo *_cur;
        friend class Addrinfo;
    };     
public:
    Addrinfo(int family, int socktype, int protocol);
    ~Addrinfo() {
        if (_result) freeaddrinfo(_result);
    }
    int getaddrinfo(const char *host, const char *service) {
        return ::getaddrinfo(host, service, &_hints, &_result);
    }
    void family(int family) {
        _hints.ai_family = family;
    }
    void socktype(int socktype) {
        _hints.ai_socktype = socktype;
    }
    void protocol(int protocol) {
        _hints.ai_protocol = protocol;
    }
    void flags(int flags) {
        _hints.ai_flags = flags;
    }
    Iterator begin() const {
        return Iterator(_result);
    }
    Iterator end() const {
        return Iterator(NULL);
    }
    const char *errinfo(int errno) const {
        return gai_strerror(errno);
    }

private:
    Addrinfo(const Addrinfo &);
    Addrinfo &operator=(const Addrinfo &);

    addrinfo *_result;
    addrinfo _hints;
};

END_NS
#endif /* ifndef __NETWORK_SOCKET_H__ */
