#include "network/tcp_socket.h"
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

BEGIN_NS(network)

static int wait(int socket, int ms);

bool TcpSocket::create(const char *host, const char *service) {
    Addrinfo ai(PF_UNSPEC, SOCK_STREAM, 0);
    if ((_errno = ai.getaddrinfo(host, service))) {
        return false;
    }

    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (socket(it->ai_family, it->ai_socktype, it->ai_protocol)
                && bind(it->ai_addr, it->ai_addrlen)
                && listen()) {
            return true;
        }
    }

	return false;
}

bool TcpSocket::connect(const char *host, const char *service) {
    Addrinfo ai(PF_UNSPEC, SOCK_STREAM, 0);
    if ((_errno = ai.getaddrinfo(host, service))) {
        return false;
    }

    for (Addrinfo::Iterator it = ai.begin(); it != ai.end(); ++it) {
        if (socket(it->ai_family, it->ai_socktype, it->ai_protocol)
                && Socket::connect(it->ai_addr, it->ai_addrlen)) {
            return true;
        }
    }

    return false;
}

bool TcpSocket::connect(const char *host, const char *service, int ms) {
	int oldFlags = setNonblock();
	if(!connect(host, service)) {
        if((errno != EINPROGRESS && errno != EWOULDBLOCK) || wait(_socket, ms) <= 0) {
            _errno = errno;
			return false;
        }
	    int error = 0;
        if(getOpt(SO_ERROR, &error) < 0 || error) {
            _errno = errno;
			return false;
        }
	}
	fcntl(_socket, F_SETFL, oldFlags);

	return true;
}

bool TcpSocket::accept(TcpSocket &client) {
	int fd = ::accept(_socket, NULL, NULL);
	if(fd < 0) {
        _errno = errno;
		return false;
    }
	client.attach(fd);
	
    return true;
}

int TcpSocket::recvn(void *buf, size_t size, int flags) {
    size_t left = size;
    char *p = (char*)buf;

    while (left > 0) {
        int nr = recv(p, left, flags);
        if (nr <= 0) {
            return nr;
        }
        left -= nr;
        p += nr;
    }
    return size - left;
}

int TcpSocket::sendn(const void *buf, size_t size, int flags) {
    size_t left = size; 
    const char *p = (const char*)buf;

    while (left > 0) {
        int ns = send(p, left, flags);
        if (ns < 0) {
            return ns;
        }
        left -= ns;
        p += ns;
    }
    return size - left;
}

bool TcpSocket::getpeername(char *addr, int addrLen, int *port) {
    sockaddr_storage peer;
    socklen_t len = sizeof(peer);
    if (::getpeername(_socket, (sockaddr*)&peer, &len) < 0) {
        _errno = errno;
        return false;
    }

    return getnameinfo(&peer, addr, addrLen, port);
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

END_NS
