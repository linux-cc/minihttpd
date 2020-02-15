#include "network/socket.h"
#include <sys/socket.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>

namespace network {

bool Socket::socket(int family, int socktype, int protocol) {
    if (_socket == -1) {
        _socket = ::socket(family, socktype, protocol);
        if (_socket < 0) {
            return false;
        }
        setOpt(SO_REUSEADDR, SOL_SOCKET, 1);
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

size_t Socket::recv(void *buf, size_t size, int flags) {
    size_t n;
    while (true) {
	    n = ::recv(_socket, buf, size, flags);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }

	return n;
}

size_t Socket::send(const void *buf, size_t size, int flags) {
    size_t n;
    while (true) {
	    n = ::send(_socket, buf, size, flags);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }

	return n;
}

size_t Socket::send(const void *buf1, size_t size1, const void *buf2, size_t size2) {
    struct iovec iov[2];
    iov[0].iov_base = (char*)buf1;
    iov[0].iov_len = size1;
    iov[1].iov_base = (char*)buf2;
    iov[1].iov_len = size2;
    size_t n;

    while (true) {
        n = writev(_socket, iov, 2);
        if (!(n < 0 && errno == EINTR)) {
            break;
        }
    }

    return n;
}

size_t Socket::send(const void *buf1, size_t size1, const void *buf2, size_t size2, const void *buf3, size_t size3) {
    struct iovec iov[3];
    iov[0].iov_base = (char*)buf1;
    iov[0].iov_len = size1;
    iov[1].iov_base = (char*)buf2;
    iov[1].iov_len = size2;
    iov[2].iov_base = (char*)buf3;
    iov[2].iov_len = size3;
    size_t n;

    while (true) {
        n = writev(_socket, iov, 3);
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
bool TcpSocket::connect(const char *host, const char *service, int ms, Family family) {
	int oldFlags = setNonblock();
	if(!connect(host, service, family)) {
        if((errno != EINPROGRESS && errno != EWOULDBLOCK) || wait(_socket, ms) <= 0) {
            return false;
        }
	    int error = 0;
        if(getOpt(SO_ERROR, SOL_SOCKET, &error) < 0 || error) {
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

int UdpSocket::sendto(const char *host, const char *service, const void *buf, size_t size, Family family, int flags) {
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

} /* namespace network */
