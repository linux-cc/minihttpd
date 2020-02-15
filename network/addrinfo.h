#ifndef __SOCKET_ADDRINFO_H__
#define __SOCKET_ADDRINFO_H__

#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>

namespace network {

class Sockaddr {
public:
    Sockaddr() {
        _len = sizeof(sockaddr_storage);
    }
    Sockaddr(const sockaddr *addr, socklen_t len): _len(len) {
        memcpy(&_addr, addr, len);
    }
    int family() const {
        return _addr.ss_family;
    }
    socklen_t &len() {
        return _len;
    }
    socklen_t len() const {
        return _len;
    }

#define OPERATOR(type)\
    operator type *() { return (type*)&_addr; }\
    operator const type *() const { return (type*)&_addr; }

    OPERATOR(sockaddr)
    OPERATOR(sockaddr_in)
    OPERATOR(sockaddr_in6)
    OPERATOR(sockaddr_storage)
    OPERATOR(sockaddr_un)
#undef OPERATOR

private:
    socklen_t _len;
    sockaddr_storage _addr;
};

class Peername {
public:
    Peername(): _port(0) {
        _name[0] = 0;
    }
    Peername(const Sockaddr &addr);
    Peername(const Peername &other) {
        *this = other;
    }
    Peername& operator=(const Peername &other);
    const char *name() const {
        return _name;
    }
    int port() const {
        return _port;
    }

private:
    char _name[sizeof(sockaddr_storage)];
    int _port;
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
    Addrinfo(int family, int socktype, int protocol, int flags = 0);
    ~Addrinfo();
    int getaddrinfo(const char *host, const char *service);

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

} /* namespace network */
#endif /* ifndef __NETWORK_ADDRINFO_H__ */
