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
    bool addHeader(const string &line);
    void parseStatusLine(const string &line);
    bool connectionClose() const;

    bool isGet() const {
        return !strncasecmp(_method.c_str(), "GET", 3);
    }
    bool isPost() const {
        return !strncasecmp(_method.c_str(), "POST", 4);
    }
    bool isHttp11() const {
        return !strncasecmp(_version.c_str(), "HTTP/1.1", 8);
    }
    char *content() {
        return (char*)_content.data();
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
    string headers() const;

private:
    string _method;
    string _uri;
    string _version;
    typedef map<string, string>::const_iterator ConstIt;
    map<string, string> _headrs;
    string _querys;
    string _content;
};

END_NS
#endif /* ifndef __HTTPD_REQUEST_H__ */
