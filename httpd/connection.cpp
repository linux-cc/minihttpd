#include "httpd/connection.h"
#include "socket/socket.h"

BEGIN_NS(httpd)

USING_CLASS(socket, TcpSocket);

Connection::Connection(int socket, int bufSize):
_socket(socket),
_recvIndex(0),
_recvBufSize(bufSize),
_sendIndex(0),
_sendBufSize(bufSize) {
    _recvBuf = new char[_recvBufSize];
    _sendBuf = new char[_sendBufSize];
}

bool Connection::recv() {
    int n = TcpSocket(_socket).recv(_recvBuf + _recvIndex, _recvBufSize - _recvIndex);
    if (n <= 0) {
        if (n < 0 && errno == EAGAIN) {
            return true;
        }
        return false;
    }
    _recvIndex += n;

    return true;
}

void Connection::adjust(const char *last) {
    int length = last - _recvBuf;
    _recvIndex -= length;
    memmove(_recvBuf, last , _recvIndex);
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

void Connection::close() {
    if (_socket != -1) {
        ::close(_socket);
        _socket = -1;
        _recvIndex = 0;
    }
}

void Connection::release() {
    close();
    _recvBufSize = 0;
    delete []_recvBuf;
    _recvBuf = NULL;
}

END_NS
