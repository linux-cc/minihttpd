#ifndef __HTTPD_RESPONSE_H__
#define __HTTPD_RESPONSE_H__

#include "config.h"
#include <unistd.h>
#include <string>
#include <map>

BEGIN_NS(httpd)

using std::string;
using std::map;
class Request;

class Response {
public:
    Response(): _cgiBin(false), _fd(-1), _contentLength(0) {}
    ~Response() { if (_fd > 0) close(_fd); }
    void parseRequest(const Request &request);
    const string &headers() {
        return _headers;
    }
    int contentLength() const {
        return _contentLength;
    }
    int fd() const {
        return _fd;
    }

private:
    int parseFile(const string &file);
    void setStatusLine(int status, const string &version);
    string parseUri(const string &uri);
    void setContentType(const string &file);
    void setContentLength(off_t length);

    bool _cgiBin;
    int _fd;
    off_t _contentLength;
    string _headers;
};

END_NS
#endif /* ifndef __HTTPD_RESPONSE_H__ */
