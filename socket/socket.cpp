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

END_NS
