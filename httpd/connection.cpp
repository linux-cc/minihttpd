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
    _LOG_("[Connection::recv]iovcnt: %d, size: %lu\n", iovcnt, n);
    
    return n >= 0 || (n < 0 && errno == EAGAIN);
}

} /* namespace httpd */
