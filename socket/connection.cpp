#include "socket/connection.h"
#include "socket/tcp_socket.h"

BEGIN_NS(socket)

Connection::Connection(int socket):
_socket(socket),
_recvIndex(0),
_sendIndex(0),
_recvBufSize(0),
_sendBudSize(0),
_recvBuf(NULL),
_sendBuf(NULL) {
}

bool Connection::init(int bufSize) {
    _recvBufSize = _sendBudSize = bufSize;
    _recvBuf = new char[_recvBufSize];
    _sendBuf = new char[_sendBudSize];
    
    return _recvBuf && _sendBuf;
}

int Connection::recv(void *buf, int size) {
    if (_recvIndex >= size) {
        return copy(buf, size);
    }
    int len = TcpSocket(_socket).recv(_recvBuf + _recvIndex, _recvBufSize - _recvIndex);
    if (len <= 0) {
        return len;
    }
    _recvIndex += len;

    return copy(buf, MIN(_recvIndex, size));
}

int Connection::send(const void *buf, int size) {
    if (_sendIndex) {
        if (_sendIndex + size > _sendBudSize) {
            errno = ENOMEM;
            return -1;
        }
        memcpy(_sendBuf + _sendIndex, buf, size);
        _sendIndex += size;
        return size;
    }
    int len = TcpSocket(_socket).send(buf, size);
    if (len <= 0) {
        if (errno == EINTR || errno == EAGAIN) {
            memcpy(_sendBuf, buf, size);
            _sendIndex += size;
            return size;
        }
        return -1;
    }
    if (len < size) {
        int left = size - len;
        memcpy(_sendBuf, (char*)buf + len, left);
        _sendIndex += left;
        return len;
    }

    return size;
}

void Connection::release() {
    close();
    _socket = -1;
    _recvIndex = 0;
    _sendIndex = 0;
    _recvBufSize = 0;
    _sendBudSize = 0;
    delete []_recvBuf;
    delete []_sendBuf;
    _recvBuf = NULL;
    _sendBuf = NULL;
}

int Connection::doPollout() {
    if (!_sendIndex) {
        return 0;
    }
    int len = TcpSocket(_socket).send(_sendBuf, _sendIndex);
    if (len <= 0 && errno != EINTR && errno != EAGAIN) {
        return len;
    }
    if (len < _sendIndex) {
        int left = _sendIndex - len;
        memmove(_sendBuf, _sendBuf + len, left);
    }
    _sendIndex -= len;

    return _sendIndex;
}

int Connection::copy(void *buf, int size) {
    memcpy(buf, _recvBuf, size);
    _recvIndex -= size;
    if (_recvIndex) {
        memmove(_recvBuf, _recvBuf + size, _recvIndex);
    }

    return size;
}

END_NS
