#ifndef __HTTPD_CONNECT_H__
#define __HTTPD_CONNECT_H__

#include "config.h"
#include <unistd.h>

BEGIN_NS(httpd)

class Connection {
public:
    Connection(int socket = -1, int bufSize = 8192);
    int recvn(void *buf, int size);
    int recvline(void *buf, int size);
    int sendn(const void *buf, size_t size);
    void release();

    operator int() {
        return _socket;
    }
    void attach(int fd) {
        _socket = fd;
    }
    void close() {
        ::close(_socket);
    }

private:
    int _socket;
    int _recvIndex;
    int _recvBufSize;
    char *_recvBuf;
};

END_NS
#endif /* ifndef __HTTPD_CONNECT_H */
