#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "config.h"
#include "httpd/header.h"
#include "utils/string_utils.h"
#include <strings.h>
#include <map>

BEGIN_NS(httpd)

USING_CLASS(utils, StringUtils);
using std::map;

class Request{
public:
    void addHeader(const Header &header);
    void parseStatusLine(const string &line);
    bool connectionClose() const;

    bool isGet() const {
        return !strncasecmp(_statusLine.method.c_str(), "GET", 3);
    }
    bool isPost() const {
        return !strncasecmp(_statusLine.method.c_str(), "POST", 4);
    }
    bool isHttp11() const {
        return !strncasecmp(_statusLine.version.c_str(), "HTTP/1.1", 8);
    }
    char *content() {
        return (char*)_content.data();
    }
    int contentLength() const {
        return _content.size();
    }
    const string &uri() const {
        return _statusLine.uri;
    }
    const string &version() const {
        return _statusLine.version;
    }

private:
    struct StatusLine {
        string method;
        string uri;
        string version;
    }_statusLine;
    typedef map<string, string>::const_iterator ConstIt;
    map<string, string> _headrs;
    map<string, string> _params;
    string _content;
};

END_NS
#endif /* ifndef __HTTPD_REQUEST_H__ */
