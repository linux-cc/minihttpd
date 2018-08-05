#include "network/tcp_socket.h"
#include <sys/select.h>
#include <fcntl.h>
#include <errno.h>

BEGIN_NS(network)

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

bool TcpSocket::accept(TcpSocket &client) {
	int fd = ::accept(_socket, NULL, NULL);
	if(fd < 0) {
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
