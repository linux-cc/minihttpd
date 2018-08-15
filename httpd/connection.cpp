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

bool Connection::send() {
    if (_sendIndex) {
        int n = TcpSocket(_socket).send(_sendBuf, _sendIndex);
        if (n < 0) {
            return errno == EAGAIN ? true : false;
        }
        _sendIndex -= n;
    }

    return true;
}

int Connection::send(const void *buf, int size) {
    TcpSocket sock(_socket);
    int n;
    if (!_sendIndex) {
        n = sock.send(buf, size);
        if (n < 0) {
            return errno == EAGAIN ? copy(buf, size) : n;
        }
        if (n < size) {
            n += copy(buf, size);
        }
        return n;
    }
    int length = copy(buf, size);
    n = sock.send(_sendBuf, _sendIndex);
    if (n < 0) {
        return errno == EAGAIN ? length : n;
    }
    _sendIndex -= n;
    if (_sendIndex) {
        memmove(_sendBuf, _sendBuf + n, _sendIndex);
    }

    return n;
}

int Connection::copy(const void *buf, int size) {
    int length = MIN(size, _sendBufSize - _sendIndex);
    if (length) {
        memcpy(_sendBuf + _sendIndex, buf, length);
        _sendIndex += length;
    }

    return length;
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

void Connection::adjust(const char *last) {
    int length = last - _recvBuf;
    if (length) {
        _recvIndex -= length;
        memmove(_recvBuf, last , _recvIndex);
    }
}

END_NS
