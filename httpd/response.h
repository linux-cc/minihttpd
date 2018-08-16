#ifndef __HTTPD_RESPONSE_H__
#define __HTTPD_RESPONSE_H__

#include "config.h"
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
    Response(): _cgiBin(false), _fd(-1), _status(PARSE_REQUEST),
        _headersPos(0), _filePos(0), _fileLength(0) {}
    void parseRequest(const Request &request);
    void setCommonHeaders(const Request &request);
    bool sendHeaders(Connection *conn);
    bool sendContent(Connection *conn);
    bool connectionClose() const;

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

    string _version;
    string _code;
    string _reason;
    bool _cgiBin;
    int _fd;
    map<int, string> _headers;
    Status _status;
    string _headersStr;
    int _headersPos;
    off_t _filePos;
    off_t _fileLength;
    typedef map<int, string>::const_iterator HeaderIt;
};

END_NS
#endif /* ifndef __HTTPD_RESPONSE_H__ */
