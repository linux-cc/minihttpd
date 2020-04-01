#ifndef __FCGID_H__
#define __FCGID_H__

#include "util/config.h"
#include "util/string.h"
#include "network/socket.h"
#include <arpa/inet.h>

namespace httpd {

using util::String;
using network::TcpSocket;
using network::Socket;
class FastCgid {
public:
    bool init(int argc, char *argv[]);
    void run();

private:
    void process();
    void waitpid(pid_t pid);

    String _cgiFile;
    TcpSocket _socket;
    int _children;
};

struct FcgiHeader {
    uint32_t _length;
    uint32_t _dataPos;
    
    void setLength(uint32_t length) { _length = htonl(length); }
    uint32_t getLength() const { return ntohl(_length); }
    void setDataPos(uint32_t dataPos) { _dataPos = dataPos; }
    uint32_t getDataPos() const { return _dataPos; }
};

int fcgiAccept();

}
#endif /* ifndef __FCGID_H__ */
