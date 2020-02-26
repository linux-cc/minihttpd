#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "util/string.h"
#include "util/buffer_queue.h"

namespace httpd {

using util::String;
using util::BufferQueue;
class Connection;
class Request {
public:
    Request();
    bool parseHeaders(Connection *conn);
    void parseContent(Connection *conn);
    
    String getHeader(const char *field) const;
    bool isMultipart() const { return _isMultipart; }
    bool is100Continue() const { return _is100Continue; }
    const String &getUri() const { return _uri; }
    const String &headers() const  { return _headers; }
    bool isGet() const { return _headers[0] == 'G' && _headers[1] == 'E' && _headers[2] == 'T'; }
    bool isPost() const { return _headers[0] == 'P' && _headers[1] == 'O' && _headers[2] == 'S' && _headers[3] == 'T'; }
    char *content() { return (char*)_content.data() + _contentPos; }
    size_t contentLength() { return _contentLength - _contentPos; }
    bool isCompleted() { return _contentPos >= _contentLength; }
    String &headerBuffer() { return _headers; }
    bool hasContent() const { return !_is100Continue && _contentLength; }
    void appendPos(size_t n) { _contentPos += n; }
    
private:
    String _headers;
    String _uri;
    String _content;
    BufferQueue _buffer;
    int _contentPos;
    int _contentLength;
    int _formFd;
    uint8_t _is100Continue : 1;
    uint8_t _isMultipart : 1;
    uint8_t _reserve : 6;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_REQUEST_H__ */
