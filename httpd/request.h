#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "util/string.h"

namespace httpd {

using util::String;
class Request {
public:
    Request();
    void parseHeaders();
    bool parseMultipart();
    
    String getHeader(const char *field) const;
    bool isMultipart() const { return _isMultipart; }
    bool is100Continue() const { return _is100Continue; }
    const String &getUri() const { return _uri; }
    const String &headers() const  { return _headers; }
    bool isGet() const { return _headers[0] == 'G' && _headers[1] == 'E' && _headers[2] == 'T'; }
    bool isPost() const { return _headers[0] == 'P' && _headers[1] == 'O' && _headers[2] == 'S' && _headers[3] == 'T'; }
    char *contentPos() { return (char*)_content.data() + _contentPos; }
    size_t contentLength() { return _contentLength - _contentPos; }
    bool isParseContentDone(size_t pos) { return (_contentPos += pos) >= _contentLength; }
    String &headerBuffer() { return _headers; }
    bool hasContent() const { return !_is100Continue && _contentLength; }
    
private:
    struct MultipartHeader {
        void parse(const String &field, const String &value);

        String name;
        String value;
        bool hasType;
    };
    size_t parseMultipart(const char *pos, const char *last, const String &boundary);
    size_t parseFormHeader(const char *pos, const char *last);
    void setMultipartName();
    size_t setMultipartContent(const char *pos, size_t length);
    int parseFormBody(const char *pos, const char *last);
    void addHeader(const String &field, const String &value);

    String _headers;
    String _uri;
    String _content;
    MultipartHeader _curMultipartHeader;
    int _contentPos;
    int _contentLength;
    int _multipartPos;
    int _multipartFd;
    uint8_t _is100Continue : 1;
    uint8_t _isMultipart : 1;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_REQUEST_H__ */
