#ifndef __HTTPD_RESPONSE_H__
#define __HTTPD_RESPONSE_H__

#include "util/string.h"
#include "httpd/gzip.h"

namespace httpd {

using util::String;
class Request;
class Connection;
class Response : public GCallback {
public:
    Response(Connection *conn = NULL):  _conn(conn), _filePos(0), _fileLength(0),
        _code(0), _fd(-1), _headPos(0), _cgiBin(0), _connClose(0), _acceptGz(0), _gzEof(0) {}
    ~Response() { if (_fd > 0 ) { close(_fd); _fd = -1; } }
    
    void parseRequest(const Request *req);
    bool sendResponse(GZip &gzip);
    bool connectionClose() const { return _connClose; }
    bool sendCompleted() const;
    const String &headers() const { return _headers; }
    
private:
    void setStatusLine(int status, const String &version);
    String parseUri(const String &uri);
    int parseFile(const String &file);
    void setContentType(const String &file);
    void setCommonHeaders(const Request *req);
    ssize_t sendFile();
    ssize_t gzflush(const void *buf, size_t len, bool eof);

    Connection *_conn;
    String _headers;
    off_t _filePos;
    off_t _fileLength;
    int _code;
    int _fd;
    int _headPos;
    uint8_t _cgiBin: 1;
    uint8_t _connClose: 1;
    uint8_t _acceptGz: 1;
    uint8_t _gzEof : 5;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_RESPONSE_H__ */
