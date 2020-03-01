#ifndef __HTTPD_CONNECT_H__
#define __HTTPD_CONNECT_H__

#include "util/buffer_queue.h"
#include "util/string.h"
#include "util/simple_list.h"
#include "util/scoped_ptr.h"
#include "memory/simple_alloc.h"
#include "network/socket.h"
#include "httpd/request.h"
#include "httpd/response.h"

namespace httpd {

using util::SimpleList;
using util::BufferQueue;
using util::ScopedPtr;
using memory::SimpleAlloc;
using network::TcpSocket;
class Request;
class Response;
class Connection {
public:
    Connection(int socket = -1): _socket(socket) {}
    bool recv();
    size_t recv(void *buf, size_t size) { return _recvQ.dequeue(buf, size); }
    bool recvLine(String &buf) { return _recvQ.dequeueUntil(buf, ONE_CRLF, false); }
    bool recvUntil(String &buf, const char *pattern, bool flush) { return _recvQ.dequeueUntil(buf, pattern, flush); }
    
    ssize_t send(const String &buf) { return send(buf.data(), buf.length()); }
    ssize_t send(const void *buf, size_t size) { return TcpSocket(_socket).send(buf, size); }
    ssize_t send(const void *buf1, size_t size1, const void *buf2, size_t size2) { return TcpSocket(_socket).send(buf1, size1, buf2, size2); }
    ssize_t send(struct iovec *iov, int iovcnt) { return TcpSocket(_socket).send(iov, iovcnt); }
    void close() { TcpSocket(_socket).close(); }

    int fd() const { return _socket; }
    void attach(int fd) { _socket = fd; }
    
    Request *getRequest() { return _req.release(); }
    void setRequest(Request *req) { _req.reset(req); }
    void addRequest(Request *req) { _reqList.push(req); }
    Request *popRequest() { Request *req; return _reqList.pop(req) ? req : NULL; }
    Response *getResponse() { return _resp.release(); }
    void setResponse(Response *resp) { _resp.reset(resp); }
    
private:
    int _socket;
    BufferQueue _recvQ;
    ScopedPtr<Request> _req;
    ScopedPtr<Response> _resp;
    SimpleList<Request*> _reqList;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_CONNECT_H */
