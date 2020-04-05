#ifndef __HTTPD_CONNECT_H__
#define __HTTPD_CONNECT_H__

#include "util/buffer.h"
#include "util/string.h"
#include "util/list.h"
#include "util/scoped_ptr.h"
#include "memory/simple_alloc.h"
#include "network/socket.h"
#include "httpd/request.h"
#include "httpd/response.h"

namespace httpd {

using util::List;
using util::Buffer;
using util::ScopedPtr;
using memory::SimpleAlloc;
using network::TcpSocket;
class Request;
class Response;
class Connection : public util::IBuffer {
public:
    Connection(int socket = -1): _recvBuf(this), _sendBuf(this), _socket(socket), _toNewIdx(-1), _toOldIdx(-1) {}
    bool recv();
    size_t recv(void *buf, size_t size) { return _recvBuf.read(buf, size); }
    bool recvLine(String &buf) { return recvUntil(buf, ONE_CRLF, false); }
    bool recvUntil(String &buf, const char *pattern, bool flush);
    
    ssize_t send(const String &buf) { return send(buf.data(), buf.length()); }
    ssize_t send(const void *buf, size_t size);
    ssize_t send(struct iovec *iov, int iovcnt);
    void close() { _socket.close(); }
    bool isClosed() const { return _socket.isClosed(); }

    int fd() const { return _socket; }
    void attach(int fd) { _socket.attach(fd); }
    
    Request *getRequest() { return _req.release(); }
    void setRequest(Request *req) { _req.reset(req); }
    void addRequest(Request *req) { _reqList.pushBack(req); }
    Request *popRequest();
    Response *getResponse() { return _resp.release(); }
    void setResponse(Response *resp) { _resp.reset(resp); }
    
    void setNewIdx(int newIdx) { _toNewIdx = newIdx; }
    void setOldIdx(int oldIdx) { _toOldIdx = oldIdx; }
    int getNewIdx() const { return _toNewIdx; }
    int getOldIdx() const { return _toOldIdx; }
    
private:
    ssize_t underflow(void *buf, size_t size) { return _socket.recv(buf, size); }
    ssize_t underflow(void *buf1, size_t size1, void *buf2, size_t size2) { return _socket.recv(buf1, size1, buf2, size2); }
    ssize_t overflow(const void *buf, size_t size) { return _socket.send(buf, size); }
    ssize_t overflow(const void *buf1, size_t size1, const void *buf2, size_t size2) { return _socket.send(buf1, size1, buf2, size2); }
    
    Buffer _recvBuf;
    Buffer _sendBuf;
    ScopedPtr<Request> _req;
    ScopedPtr<Response> _resp;
    List<Request*> _reqList;
    TcpSocket _socket;
    int _toNewIdx;
    int _toOldIdx;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_CONNECT_H */
