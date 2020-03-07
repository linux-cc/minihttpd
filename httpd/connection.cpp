#include "httpd/connection.h"
#include <errno.h>

namespace httpd {

bool Connection::recv() {
    struct iovec iov[2];
    int iovcnt = _recvQ.getWriteIov(iov);
    if (!iovcnt) {
        return false;
    }
    
    ssize_t n = _socket.recv(iov, iovcnt);
    if (n < 0 && errno != EAGAIN) {
        return false;
    }
    _recvQ.setWritePos(n);
    
    return n;
}

ssize_t Connection::send(const void *buf, size_t size) {
    struct iovec iov[3];
    int iovcnt = _sendQ.getReadIov(iov);
    iov[iovcnt].iov_base = (void*)buf;
    iov[iovcnt].iov_len = size;
    iovcnt++;
    ssize_t n = _socket.send(iov, iovcnt);
    if (n < 0 && errno != EAGAIN) {
        return n;
    }
    size_t qlen = _sendQ.length();
    if (n < qlen) {
        _sendQ.setReadPos(n);
        return n + _sendQ.enqueue(buf, size);
    }
    _sendQ.setReadPos(qlen);
    size_t slen = n - qlen;
    if (slen < size) {
        return n + _sendQ.enqueue((char*)buf + slen, size - slen);
    }
    
    return n;
}

ssize_t Connection::send(struct iovec *iov, int iovcnt) {
    struct iovec _iov[MAX_IOV_CNT + 2];
    int _iovcnt = _sendQ.getReadIov(_iov);
    for (int i = 0; i < iovcnt && i < MAX_IOV_CNT; ++i) {
        _iov[_iovcnt+i].iov_base = iov[i].iov_base;
        _iov[_iovcnt+i].iov_len = iov[i].iov_len;
    }
    _iovcnt += iovcnt;
    
    size_t slen = 0;
    ssize_t n = _socket.send(_iov, _iovcnt);
    int i = 0;
    for (; i < _iovcnt; ++i) {
        slen += _iov[i].iov_len;
        if (n < slen) break;
    }
    if (i == _iovcnt) {
        return n;
    }
    
    size_t qlen = _sendQ.length();
    _sendQ.setReadPos(n < qlen ? n : qlen);
    size_t rest = slen - n;
    size_t nq = 0;
    if (rest) {
        nq += _sendQ.enqueue((char*)iov[i].iov_base + (_iov[i].iov_len - rest), rest);
    }
    for (++i; i < _iovcnt; i++) {
        nq += _sendQ.enqueue(iov[i].iov_base, iov[i].iov_len);
    }
    
    return n + nq;
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
