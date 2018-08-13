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
    Response(): _cgiBin(false), _fd(-1) {}
    ~Response() { if (_fd > 0) close(_fd); }
    void parseRequest(const Request &request);
    string headers() const;
    bool connectionClose() const;
    int contentLength() const;
    int fd() const {
        return _fd;
    }

private:
    string parseUri(const string &uri);
    int parseFile(const string &file);
    void setStatusLine(int status, const string &version);
    void setContentType(const string &file);
    void setContentLength(off_t length);
    void setConnection(const Request &request);
    void setServer();

    string _version;
    string _status;
    string _reason;
    bool _cgiBin;
    int _fd;
    typedef map<string, string>::const_iterator ConstIt;
    map<string, string> _headers;
};

END_NS
#endif /* ifndef __HTTPD_RESPONSE_H__ */
