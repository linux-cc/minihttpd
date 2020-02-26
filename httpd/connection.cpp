#include "httpd/connection.h"
#include <errno.h>

namespace httpd {

using network::TcpSocket;

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

bool Connection::send() {
    ssize_t n = 0;
    struct iovec iov[2];
    int iovcnt = _sendQ.getReadIov(iov);
    if (iovcnt) {
        n = TcpSocket(_socket).send(iov, iovcnt);
        if (n > 0) {
            _sendQ.setReadPos(n);
        }
    }
    
    return n > 0 || (n < 0 && errno == EAGAIN);
}

Request *Connection::getRequest() {
    if (!_req) {
        _req.reset(memory::SimpleAlloc<Request>::New());
    }
    return _req.get();
}

Response *Connection::getResponse() {
    if (!_resp) {
        _resp = makeScopedPtr(memory::SimpleAlloc<Response>::New());
    }
    return _resp.get();
}

} /* namespace httpd */
