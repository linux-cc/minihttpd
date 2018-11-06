#ifndef __HTTPD_CONNECT_H__
#define __HTTPD_CONNECT_H__

#include "httpd/request.h"
#include "httpd/response.h"
#include <unistd.h>

namespace httpd {

class Connection {
public:
    Connection(int socket = -1, int bufSize = (1 << 14));
    bool recv();
    bool send();
    bool send(const void *buf, int size);
    bool send(const void *buf1, int size1, const void *buf2, int size2);
    bool send(const void *buf1, int size1, const void *buf2, int size2, const void *buf3, int size3);
    void close();
    void release();
    void seek(const char *pos);

    const char *begin() const {
        return _recvBuf;
    }
    const char *end() const {
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
    bool copy(const void *buf, int size, int sendn);
    bool copy(const void *buf1, int size1, const void *buf2, int size2, int sendn);

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

} /* namespace httpd */
#endif /* ifndef __HTTPD_CONNECT_H */
