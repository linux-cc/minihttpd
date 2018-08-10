#include "socket/socket.h"
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

BEGIN_NS(socket)

bool Socket::socket(int family, int socktype, int protocol) {
    if (_socket == -1) {
        _socket = ::socket(family, socktype, protocol);
        if (_socket < 0) {
            return false;
        }
        setOpt(SO_REUSEADDR, 1);
    }

    return true;
}

bool Socket::create(const char *host, const char *service, int family, int socktype, int protocol) {
    Addrinfo ai(family, socktype, protocol, AI_PASSIVE);
    if ((errno = ai.getaddrinfo(host, service))) {
        return false;
    }

    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (socket(it->ai_family, it->ai_socktype, it->ai_protocol)
                && bind(_socket, it->ai_addr, it->ai_addrlen) == 0) {
            return socktype == SOCK_STREAM ? listen(_socket, 8) == 0 : true;
        }
    }
    close();

	return false;
}

bool Socket::connect(const char *host, const char *service, int family, int socktype, int protocol) {
    Addrinfo ai(family, socktype, protocol);
    if ((errno = ai.getaddrinfo(host, service))) {
        return false;
    }

    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (socket(it->ai_family, it->ai_socktype, it->ai_protocol)
                && ::connect(_socket, it->ai_addr, it->ai_addrlen) == 0) {
            return true;
        }
    }

    return false;
}

int Socket::recv(void *buf, size_t size, int flags) {
    int n;
    while (true){
	    n = ::recv(_socket, buf, size, flags);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }

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

	return n;
}

int Socket::setNonblock() {
	int flags = fcntl(_socket, F_GETFL);
	fcntl(_socket, F_SETFL, flags | O_NONBLOCK);

    return flags;
}

void Socket::close() {
	if(_socket != -1) {
        ::close(_socket);
        _socket = -1;
	}
}

static int wait(int socket, int ms);
bool TcpSocket::connect(const char *host, const char *service, int ms, int family) {
	int oldFlags = setNonblock();
	if(!connect(host, service, family)) {
        if((errno != EINPROGRESS && errno != EWOULDBLOCK) || wait(_socket, ms) <= 0) {
            return false;
        }
	    int error = 0;
        if(getOpt(SO_ERROR, &error) < 0 || error) {
            return false;
        }
	}
	fcntl(_socket, F_SETFL, oldFlags);

	return true;
}

int wait(int socket, int ms) {
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

int UdpSocket::sendto(const char *host, const char *service, const void *buf, size_t size, int family, int flags) {
    Addrinfo ai(family, SOCK_DGRAM, 0);
    if (ai.getaddrinfo(host, service)) {
        return -1;
    }
    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (!socket(it->ai_family, it->ai_socktype, it->ai_protocol)) {
            continue;
        }
        return Socket::sendto(buf, size, Sockaddr(it->ai_addr, it->ai_addrlen), flags);
    }

    return -1;
}

END_NS
