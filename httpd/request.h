#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "config.h"
#include <strings.h>
#include <string>
#include <map>

BEGIN_NS(httpd)

using std::string;
using std::map;

class Request {
public:
    enum Status {
        PROCESS_LINE,
        PROCESS_HEADERS,
        PROCESS_CONTENT,
        PROCESS_DONE,
    };
    Request(): _status(PROCESS_LINE), _contentIndex(0) {}
    void parseStatusLine(const char *beg, const char *end);
    void addHeader(const char *beg, const char *end);
    int setContent(const char *beg, const char *end);
    void reset(bool is100Continue);
    string headers() const;
    const string *getHeader(int field) const;
    const string *getConnection() const;
    bool has100Continue() const;

    bool isGet() const {
        return !strncasecmp(_method.c_str(), "GET", 3);
    }
    bool isPost() const {
        return !strncasecmp(_method.c_str(), "POST", 4);
    }
    bool isHttp11() const {
        return !strncasecmp(_version.c_str(), "HTTP/1.1", 8);
    }
    const char *content() const {
        return _content.data();
    }
    int contentLength() const {
        return _content.size();
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
    bool inProcessHeaders() const {
        return _status == PROCESS_HEADERS;
    }
    bool inProcessContent() const {
        return _status == PROCESS_CONTENT;
    }

private:
    string _method;
    string _uri;
    string _version;
    typedef map<string, string>::const_iterator ConstIt;
    map<string, string> _headers;
    string _querys;
    string _content;
    Status _status;
    int _contentIndex;
};

END_NS
#endif /* ifndef __HTTPD_REQUEST_H__ */
