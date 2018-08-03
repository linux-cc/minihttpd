#include "network/socket.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

BEGIN_NS(network)

static int wait(int socket, int ms);

bool Socket::socket(int family, int socktype, int protocol) {
    close();
    _socket = ::socket(family, socktype, protocol);
    if (_socket < 0) {
        _errno = errno;
        return false;
    }

    setOpt(SO_REUSEADDR, 1);
    return true;
}

bool Socket::bind(sockaddr *addr, socklen_t len) {
    if (::bind(_socket, addr, len)) {
        _errno = errno;
        close();
        return false;
    }

    return true;
}

bool Socket::listen(int backlog) {
    if (::listen(_socket, backlog)) {
        _errno = errno;
        close();
        return false;
    }

    return true;
}

bool Socket::connect(sockaddr *addr, socklen_t len) {
    if(::connect(_socket, addr, len)) {
        _errno = errno;
        close();
        return false;
    }

    return true;
}

int Socket::recv(void *buf, size_t size, int flags) {
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

int Socket::send(const void *buf, size_t size, int flags) {
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

int Socket::recvfrom(void *buf, size_t size, char *addr, int addrLen, int *port, int flags) {
    sockaddr_storage peer;
    socklen_t slen = sizeof(peer);
    int len = ::recvfrom(_socket, buf, size, flags, (sockaddr*)&peer, &slen);
    _errno = errno;
    getnameinfo(&peer, addr, addrLen, port);
    return len;
}

int Socket::sendto(const void *buf, size_t size, const sockaddr *addr, socklen_t addrlen, int flags) {
    int len = ::sendto(_socket, buf, size, flags, addr, addrlen);
    _errno = errno;
    return len;
}

int Socket::setNonblock() {
	int flags = fcntl(_socket, F_GETFL);
	fcntl(_socket, F_SETFL, flags | O_NONBLOCK);
    _errno = errno;
    return flags;
}

bool Socket::getnameinfo(const sockaddr_storage *srcAddr, char *dstAddr, int dstLen, int *port) {
    void *inaddr;
    if (srcAddr->ss_family == PF_INET) {
        inaddr = &((sockaddr_in*)srcAddr)->sin_addr;
        *port = ntohs(((sockaddr_in*)srcAddr)->sin_port);
    } else if (srcAddr->ss_family == PF_INET6) {
        inaddr = &((sockaddr_in6*)srcAddr)->sin6_addr;
        *port = ntohs(((sockaddr_in6*)srcAddr)->sin6_port);
    } else {
        return false;
    }
    if (!inet_ntop(srcAddr->ss_family, inaddr, dstAddr, dstLen)) {
        _errno = errno;
        return false;
    }
    return true;
}

void Socket::close() {
	if(_socket != -1) {
		::close(_socket);
        _errno = errno;
		_socket = -1;
	}
}

Addrinfo::Addrinfo(int family, int socktype, int protocol): _result(NULL) {
    memset(&_hints, 0, sizeof(_hints));
    _hints.ai_family = family;
    _hints.ai_socktype = socktype;
    _hints.ai_protocol = protocol;
}

END_NS
