#ifndef __SOCKET_CONNECT_H__
#define __SOCKET_CONNECT_H__

#include "config.h"
#include <unistd.h>

BEGIN_NS(socket)

class TcpSocket;

class Connection {
public:
    explicit Connection(int socket = -1);
    bool init(int bufSize = 8192);
    int recv(void *buf, int size);
    int send(const void *buf, int size); 
    int doPollout();
    void release();

    operator int() {
        return _socket;
    }
    bool isPollout() {
        return _sendIndex;
    }
    void attach(int fd) {
        _socket = fd;
    }
    void close() {
        ::close(_socket);
    }

private:
    int copy(void *buf, int size);
    int copy(const void *buf, int size);

    int _socket;
    int _recvIndex;
    int _sendIndex;
    int _recvBufSize;
    int _sendBudSize;
    char *_recvBuf;
    char *_sendBuf;
};

END_NS
#endif /* ifndef __SOCKET_CONNECT_H


 */
