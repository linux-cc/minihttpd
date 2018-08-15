#ifndef __HTTPD_CONNECT_H__
#define __HTTPD_CONNECT_H__

#include "config.h"
#include "httpd/request.h"
#include "httpd/response.h"
#include <unistd.h>

BEGIN_NS(httpd)

class Connection {
public:
    Connection(int socket = -1, int bufSize = 8192);
    bool recv();
    bool send();
    int send(const void *buf, int size);
    void close();
    void release();
    void adjust(const char *last);

    const char *pos() const {
        return _recvBuf;
    }
    const char *last() const {
        return _recvBuf + _recvIndex;
    }
    operator int() {
        return _socket;
    }
    void attach(int fd) {
        _socket = fd;
    }
    Request &request() {
        return _request;
    }
    Response &response() {
        return _response;
    }
    bool needPollOut() const {
        return _sendIndex;
    }

private:
    int copy(const void *buf, int size);

    int _socket;
    int _recvIndex;
    int _recvBufSize;
    int _sendIndex;
    int _sendBufSize;
    char *_recvBuf;
    char *_sendBuf;
    Request _request;
    Response _response;
};

END_NS
#endif /* ifndef __HTTPD_CONNECT_H */
