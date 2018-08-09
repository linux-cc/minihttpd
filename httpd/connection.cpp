#include "httpd/connection.h"
#include "socket/socket.h"

BEGIN_NS(httpd)

USING_CLASS(socket, TcpSocket);

Connection::Connection(int socket, int bufSize):
_socket(socket),
_recvIndex(0),
_recvBufSize(bufSize) {
    _recvBuf = new char[_recvBufSize];
}

int Connection::recvn(void *buf, int size) {
    if (!_recvIndex) {
        return TcpSocket(_socket).recv(buf, size, MSG_WAITALL);
    }
    if (size <= _recvIndex) {
        memcpy(buf, _recvBuf, size);
        _recvIndex -= size;
        return size;
    }
    char *pb = (char*)buf;
    memcpy(pb, _recvBuf, _recvIndex);
    pb += _recvIndex;
    size -= _recvIndex;
    _recvIndex = 0;

    return TcpSocket(_socket).recv(pb, size, MSG_WAITALL);
}

int Connection::recvline(void *buf, int size) {
    if (_recvIndex < size) {
        int len = TcpSocket(_socket).recv(_recvBuf + _recvIndex, _recvBufSize - _recvIndex);
        if (len <= 0) {
            return len;
        }
        _recvIndex += len;
    }
    int i = 0, _size = MIN(_recvIndex, size) - 1;
    char *p = (char*)buf;
    for (; i < _size; ++i) {
        if ((p[i] = _recvBuf[i]) == '\n') {
            ++i;
            break;
        }
    }
    p[i] = '\0';
    _recvIndex -= i;
    memmove(_recvBuf, _recvBuf + i, _recvIndex);
    
    return i;
}

int Connection::sendn(const void *buf, size_t size) {
    size_t left = size; 
    const char *p = (const char*)buf;

    while (left > 0) {
        int ns = TcpSocket(_socket).send(p, left);
        if (ns < 0) {
            return ns;
        }
        left -= ns;
        p += ns;
    }
    return size - left;
}

void Connection::release() {
    close();
    _socket = -1;
    _recvIndex = 0;
    _recvBufSize = 0;
    delete []_recvBuf;
    _recvBuf = NULL;
}

END_NS
