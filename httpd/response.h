#ifndef __HTTPD_RESPONSE_H__
#define __HTTPD_RESPONSE_H__

#include "config.h"
#include "httpd/gzip.h"
#include <sys/stat.h>
#include <string>
#include <map>

BEGIN_NS(httpd)

using std::string;
using std::map;
class Request;
class Connection;

class Response {
public:
    enum Status {
        PARSE_REQUEST,
        SEND_HEADERS,
        SEND_CONTENT,
        SEND_DONE,
    };
    Response() {
        reset();
    }
    void parseRequest(const Request &request);
    void setCommonHeaders(const Request &request);
    bool sendHeaders(Connection *conn);
    bool sendContent(Connection *conn) {
        return _hasGzip ? sendContentChunked(conn) : sendContentOriginal(conn);
    }
    bool connectionClose() const {
        return _connClose;
    }
    bool is100Continue() const {
        return _code[0] == '1' && _code[1] == '0' && _code[2] == '0';
    }
    const char *headers() const {
        return _headersStr.c_str();
    }
    Status status() const {
        return _status;
    }
    bool inSendHeaders() const {
        return _status == SEND_HEADERS;
    }
    bool inSendContent() const {
        return _status == SEND_CONTENT;
    }
    void reset();

private:
    void setStatusLine(int status, const string &version);
    string parseUri(const string &uri);
    int parseFile(const string &file);
    void setContentInfo(const string &file, const struct stat &st);
    void setContentType(const string &file);
    void setHeadersStr();
    bool sendContentOriginal(Connection *conn);
    bool sendContentChunked(Connection *conn);

    string _version;
    string _code;
    string _reason;
    int _fd;
    Status _status;
    string _headersStr;
    int _headersPos;
    off_t _filePos;
    off_t _fileLength;
    uint8_t _cgiBin: 1;
    uint8_t _connClose: 1;
    uint8_t _hasGzip: 1;
    uint8_t _reserve: 5;
    typedef map<int, string>::const_iterator HeaderIt;
    map<int, string> _headers;
    GZip _gzip;
};

END_NS
#endif /* ifndef __HTTPD_RESPONSE_H__ */
