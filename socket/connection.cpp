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

void Connection::init(int bufSize) {
    _recvBufSize = _sendBudSize = bufSize;
    _recvBuf = new char[_recvBufSize];
    _sendBuf = new char[_sendBudSize];
}

int Connection::recv(void *buf, int size) {
    int _size = MIN(_recvBufSize, size);
    if (_recvIndex < _size) {
        int len = TcpSocket(_socket).recv(_recvBuf + _recvIndex, _recvBufSize - _recvIndex);
        if (len <= 0) {
            return len;
        }
        _recvIndex += len;
    }
    _size = MIN(_recvIndex, size);
    memcpy(buf, _recvBuf, _size);
    _recvIndex -= _size;
    if (_recvIndex) {
        memmove(_recvBuf, _recvBuf + _size, _recvIndex);
    }
    
    return _size;
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

int Connection::send(const void *buf, int size) {
    if (!_sendIndex) {
        int len = TcpSocket(_socket).send(buf, size);
        if (len == size) {
            return len;
        }
        if (len <= 0) {
            if (errno == EINTR || errno == EAGAIN) {
                return copy(buf, size);
            }
            return len;
        }
        return copy((char*)buf + len, size - len);
    }

    return copy(buf, size);
}

bool Connection::doPollOut() {
    if (_sendIndex) {
        int len = TcpSocket(_socket).send(_sendBuf, _sendIndex);
        if (len <= 0) {
            return errno == EINTR && errno == EAGAIN;
        }
        if (len < _sendIndex) {
            memmove(_sendBuf, _sendBuf + len, _sendIndex - len);
        } else if (_observer) {
            _observer->notifyPollOutFinish(this);
        }
        _sendIndex -= len;
    }

    return true;
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

int Connection::copy(const void *buf, int size) {
    int _size = MIN(_sendBudSize - _sendIndex, size);
    if (_size) {
        memcpy(_sendBuf + _sendIndex, buf, _size);
        _sendIndex += _size;
    }
    if (_observer) {
        _observer->notifyPollOut(this);
    }

    return _size;
}

END_NS
