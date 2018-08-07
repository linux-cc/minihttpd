#ifndef __SOCKET_CONNECT_H__
#define __SOCKET_CONNECT_H__

#include "config.h"
#include <unistd.h>

BEGIN_NS(socket)

class TcpSocket;
class Connection;

class PolloutObserver {
public:
    virtual ~PolloutObserver() {}
    virtual void notifyPollOut(Connection *conn) = 0;
    virtual void notifyPollOutFinish(Connection *conn) = 0;
};

class Connection {
public:
    explicit Connection(int socket = -1);
    bool init(int bufSize = 8192);
    int recv(void *buf, int size);
    int recvline(void *buf, int size);
    int send(const void *buf, int size); 
    bool doPollOut();
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
    void setObserve(PolloutObserver *observer) {
        _observer = observer;
    }

private:
    int copy(const void *buf, int size);

    int _socket;
    int _recvIndex;
    int _sendIndex;
    int _recvBufSize;
    int _sendBudSize;
    char *_recvBuf;
    char *_sendBuf;
    PolloutObserver *_observer;
};

END_NS
#endif /* ifndef __SOCKET_CONNECT_H


 */
