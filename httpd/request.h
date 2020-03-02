#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "util/string.h"
#include "util/scoped_ref.h"
#include "memory/simple_alloc.h"

namespace httpd {

using util::String;
using util::ScopedRef;
using util::RefCounted;
class Connection;
class Request {
public:
    Request(Connection *conn = NULL) { _ptr = memory::SimpleAlloc<Content>::New(conn); }
    bool parseHeaders();
    void parseContent();
    
    String getHttpVersion() const;
    String getHeader(const char *field) const;
    bool isMultipart() const { return _ptr->_isMultipart; }
    bool is100Continue() const { return _ptr->_is100Continue && _ptr->_status == Parse_Form_Header; }
    const String &getUri() const { return _ptr->_uri; }
    const String &headers() const  { return _ptr->_headers; }
    bool isGet() const { return _ptr->_headers[0] == 'G' && _ptr->_headers[1] == 'E' && _ptr->_headers[2] == 'T'; }
    bool isPost() const { return _ptr->_headers[0] == 'P' && _ptr->_headers[1] == 'O' && _ptr->_headers[2] == 'S' && _ptr->_headers[3] == 'T'; }
    bool isCompleted() const { return _ptr->_contentPos >= _ptr->_contentLength; }
    bool inParseHeaders() const { return _ptr->_status == Parse_Header; }
    bool inParseContent() const { return _ptr->_status > Parse_Header && _ptr->_status < Parse_Finish; }
    
private:
    bool parseFormHeader();
    bool parseFormContent();
    
    enum Status {
        Parse_Header,
        Parse_Content,
        Parse_Form_Header,
        Parse_Form_Content,
        Parse_Finish,
    };
    
    class Content : public RefCounted<Content> {
    public:
        Content(Connection *conn): _conn(conn), _contentPos(0), _contentLength(0), _formFd(-1),
            _is100Continue(0), _isMultipart(0), _status(Parse_Header) {}
        Connection *_conn;
        String _headers;
        String _uri;
        String _content;
        String _boundary;
        int _contentPos;
        int _contentLength;
        int _formFd;
        uint8_t _is100Continue : 1;
        uint8_t _isMultipart : 1;
        uint8_t _status : 6;
    private:
        ~Content() {}
        friend class memory::SimpleAlloc<Content>;
    };
    
    ScopedRef<Content> _ptr;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_REQUEST_H__ */
