#ifndef __HTTPD_RESPONSE_H__
#define __HTTPD_RESPONSE_H__

#include "config.h"
#include <string>
#include <map>

BEGIN_NS(httpd)

using std::string;
using std::map;
class Request;

class Response {
public:
    enum Status {
        PARSE_REQUEST,
        SEND_HEADERS,
        SEND_CONTENT,
        SEND_DONE,
    };
    Response(): _cgiBin(false), _fd(-1), _status(PARSE_REQUEST),
        _headersPos(0), _filePos(0), _contentLength(0) {}
    void parseRequest(const Request &request);
    bool connectionClose() const;
    const char *headers() const {
        return _headersStr.data() + _headersPos;
    }
    const char *originHeaders() const {
        return _headersStr.c_str();
    }
    int headersLength() const {
        return _headersStr.size() - _headersPos;
    }
    int headersPos() const {
        return _headersPos;
    }
    void addHeadersPos(int pos);
    int contentLength() const {
        return _contentLength - _filePos;
    }
    int fileFd() const {
        return _fd;
    }
    void addFilePos(off_t pos) {
        _filePos += pos;
        if (_filePos >= _contentLength) {
            _status = SEND_DONE;
        }
    }
    off_t filePos() const {
        return _filePos;
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
    string parseUri(const string &uri);
    int parseFile(const string &file);
    void setStatusLine(int status, const string &version);
    void setContentType(const string &file);
    void setContentLength(off_t length);
    void setConnection(const Request &request);
    void setServer();
    void setHeaders();

    string _version;
    string _code;
    string _reason;
    bool _cgiBin;
    int _fd;
    typedef map<string, string>::const_iterator ConstIt;
    map<string, string> _headers;
    Status _status;
    string _headersStr;
    int _headersPos;
    off_t _filePos;
    off_t _contentLength;
};

END_NS
#endif /* ifndef __HTTPD_RESPONSE_H__ */
