#ifndef __HTTPD_TCP_SERVER_H__
#define __HTTPD_TCP_SERVER_H__

#include "config.h"
#include "socket/tcp_socket.h"

USING_CLASS(socket, TcpSocket);

BEGIN_NS(httpd)

class TcpServer {
public:
    operator int () {
        return _server;
    }
private:
    TcpSocket _server;
};

END_NS
#endif /* ifndef __HTTPD_TCP_SERVER_H__ */
