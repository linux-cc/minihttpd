#include "httpd/connection.h"
#include "network/socket.h"

namespace httpd {

using network::TcpSocket;

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
        if (_sendIndex) {
            memmove(_sendBuf, _sendBuf + n, _sendIndex);
        }
    }

    return true;
}

bool Connection::send(const void *buf, int size) {
    if (_sendIndex) {
        return copy(buf, size, 0) && send();
    }

    TcpSocket sock(_socket);
    int n = sock.send(buf, size);
    if (n < 0 && errno != EAGAIN) {
        return false;
    }
    return copy(buf, size, n < 0 ? 0 : n);
}

bool Connection::send(const void *buf1, int size1, const void *buf2, int size2) {
    if (_sendIndex) {
        return copy(buf1, size1, 0) && send(buf2, size2);
    }

    TcpSocket sock(_socket);
    int n = sock.send(buf1, size1, buf2, size2);
    if (n < 0 && errno != EAGAIN) {
        return false;;
    }

    return copy(buf1, size1, buf2, size2, n < 0 ? 0 : n);
}

bool Connection::send(const void *buf1, int size1, const void *buf2, int size2, const void *buf3, int size3) {
    if (_sendIndex) {
        return copy(buf1, size1, 0) && send(buf2, size2, buf3, size3);
    }

    TcpSocket sock(_socket);
    int n = sock.send(buf1, size1, buf2, size2, buf3, size3);
    if (n < 0) {
        if (errno != EAGAIN) {
            return false;;
        }
        return copy(buf1, size1, buf2, size2, 0) && copy(buf3, size3, 0);
    }

    if (n < size1) {
        return copy(buf1, size1, n) && copy(buf2, size2, buf3, size3, 0);
    } else {
        return copy(buf2, size2, buf3, size3, n - size1);
    }
}

bool Connection::copy(const void *buf, int size, int sendn) {
    int left = size - sendn;
    if (left > _sendBufSize - _sendIndex) {
        errno = ENOMEM;
        return false;
    }
    if (left > 0) {
        char *pos = (char*)buf + sendn;
        memcpy(_sendBuf + _sendIndex, pos, left);
        _sendIndex += left;
    }

    return true;
}

bool Connection::copy(const void *buf1, int size1, const void *buf2, int size2, int sendn) {
    int left1 = size1 - sendn;

    if (left1 > 0) {
        return copy(buf1, size1, sendn) && copy(buf2, size2, 0);
    } else {
        return copy(buf2, size2, -left1);
    }
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
    _sendBufSize = 0;
    delete []_sendBuf;
    _sendBuf = NULL;
}

void Connection::seek(size_t length) {
    if (length) {
        _recvIndex -= length;
        memmove(_recvBuf, _recvBuf + length , _recvIndex);
    }
}

} /* namespace httpd */
