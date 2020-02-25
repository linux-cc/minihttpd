#ifndef __HTTPD_CONNECT_H__
#define __HTTPD_CONNECT_H__

#include "util/buffer_queue.h"
#include "util/string.h"
#include "util/scoped_ptr.h"
#include "httpd/request.h"
#include "httpd/response.h"
#include "network/socket.h"
#include <unistd.h>

namespace httpd {

using namespace util;
class Connection {
public:
    Connection(int socket = -1): _socket(socket) {}
    bool recv();
    bool recvLine(String &buf) { return _recvQ.dequeueUntil(buf, CRLF); }
    bool recvUntil(String &buf, const char *pattern) { return _recvQ.dequeueUntil(buf, pattern); }
    size_t recv(void *buf, size_t size) { return _recvQ.dequeue(buf, size); }
    
    bool send();
    bool send(const String &buf) { return send(buf.data(), buf.length()); }
    bool send(const void *buf, size_t size) { return _sendQ.enqueue(buf, size); }
    bool sendCompleted() { return _sendQ.empty(); }
    void close() { network::TcpSocket(_socket).close(); }

    int fd() const { return _socket; }
    void attach(int fd) { _socket = fd; }
    
    void setRequest(Request *req) { _req.reset(req); }
    void setResponse(Response *resp) { _resp.reset(resp); }
    Request *getRequest();
    Response *getResponse();
    
private:
    int _socket;
    BufferQueue _recvQ;
    BufferQueue _sendQ;
    ScopedPtr<Request> _req;
    ScopedPtr<Response> _resp;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_CONNECT_H */
