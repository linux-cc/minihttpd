#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "config.h"
#include "httpd/constants.h"
#include <strings.h>
#include <string>
#include <map>

BEGIN_NS(httpd)

using std::string;
using std::map;
class Connection;

class Request {
public:
    enum Status {
        PARSE_LINE,
        PARSE_HEADERS,
        PARSE_CONTENT,
        PARSE_DONE,
    };
    Request(): _status(PARSE_LINE), _contentPos(0) {}
    int parseStatusLine(const char *pos);
    int parseHeaders(const char *pos);
    int parseContent(const char *pos, const char *last);
    void reset(bool is100Continue);
    string headers() const;
    const string *getHeader(int field) const;
    bool has100Continue() const;

    const string &content() const {
        return _content;
    }
    const string *getConnection() const {
        return getHeader(Header::Connection);
    }
    bool isGet() const {
        return !strncasecmp(_method.c_str(), "GET", 3);
    }
    bool isPost() const {
        return !strncasecmp(_method.c_str(), "POST", 4);
    }
    bool isHttp11() const {
        return !strncasecmp(_version.c_str(), "HTTP/1.1", 8);
    }
    const string &uri() const {
        return _uri;
    }
    const string &version() const {
        return _version;
    }
    Status status() const {
        return _status;
    } 
    bool inParseStatusLine() const {
        return _status == PARSE_LINE;
    }
    bool inParseHeaders() const {
        return _status == PARSE_HEADERS;
    }
    bool inParseContent() const {
        return _status == PARSE_CONTENT;
    }

private:
    string _method;
    string _uri;
    string _version;
    typedef map<string, string>::const_iterator HeaderIt;
    map<string, string> _headers;
    string _querys;
    string _content;
    Status _status;
    int _contentPos;
};

END_NS
#endif /* ifndef __HTTPD_REQUEST_H__ */
