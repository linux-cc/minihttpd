#include "httpd/connection.h"
#include <errno.h>

namespace httpd {

bool Connection::recv() {
    ssize_t n = 0;
    struct iovec iov[2];
    
    int iovcnt = _recvQ.getWriteIov(iov);
    if (iovcnt) {
        n = TcpSocket(_socket).recv(iov, iovcnt);
        if (n > 0) {
            _recvQ.setWritePos(n);
        }
    }
    
    return n > 0 || (n < 0 && errno == EAGAIN);
}

Request *Connection::popRequest() {
    if (_reqList.empty()) {
        return NULL;
    }
    
    Request *req = _reqList.front();
    _reqList.pop_front();
    return req;
}

} /* namespace httpd */
