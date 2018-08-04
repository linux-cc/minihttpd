#ifndef __NETWORK_SOCKET_H__
#define __NETWORK_SOCKET_H__

#include "network/addrinfo.h"
#include <errno.h>

BEGIN_NS(network)

class Socket {
public:
    class Peer {
    public:
        typedef sockaddr SA;
        typedef sockaddr_in SA_I;
        typedef sockaddr_in6 SA_I6;
        typedef sockaddr_storage SA_S;
        
        Peer() { _addr._l = sizeof(SA_S); }
        Peer(const SA *addr, socklen_t len) { memcpy(&_addr._a, addr, len); _addr._l = len; }
        int family() const { return _addr._a.ss_family; }
        template <typename T> T *addr() { return (T*)&_addr._a; }
        template <typename T> const T *addr() const { return (T*)&_addr._a; }
        socklen_t &socklen() { return _addr._l; }
        socklen_t socklen() const { return _addr._l; }

        operator char *() { return _name._n; }
        operator const char *() const { return _name._n; }
        int namelen () const { return sizeof(SA_S); }
        void port(int port) { _name._p = port; }
        int port() const { return _name._p; }

    private:
        union {
            struct {
                int _p;
                char _n[sizeof(SA_S)];
            }_name;
            struct {
                socklen_t _l;
                SA_S _a;
            }_addr;
        };
    };
    
    int recv(void *buf, size_t size, int flags = 0);
    int send(const void *buf, size_t size, int flags = 0);
    int recvfrom(void *buf, size_t size, Peer &peer, int flags = 0) {
        return ::recvfrom(_socket, buf, size, flags, peer.addr<Peer::SA>(), &peer.socklen());
    }
    int sendto(const void *buf, size_t size, const Peer &peer, int flags = 0) {
        return ::sendto(_socket, buf, size, flags, peer.addr<Peer::SA>(), peer.socklen());
    }
    int setNonblock();
    bool getnameinfo(const Peer &addr, Peer &name);
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
