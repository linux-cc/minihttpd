#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "config.h"
#include "httpd/header.h"
#include "httpd/status_line.h"
#include <map>

BEGIN_NS(httpd)

using std::map;

class Request{
public:
    Request() {}
    void addHeader(const Header &header);
    void decodeContent() {}
    void parseStatusLine(const string &line) {
        _statusLine.parse(line);
        parseUriParams();
    }
    bool isGet() const {
        return _statusLine.isGet();
    }
    bool isPost() const {
        return _statusLine.isPost();
    }
    char *content() {
        return (char*)_content.data();
    }
    int contentLength() const {
        return _content.size();
    }
private:
    void parseUriParams();

    RequestStatusLine _statusLine;
    typedef map<string, string>::const_iterator ConstIt;
    map<string, string> _headrs;
    map<string, string> _params;
    string _content;
};

END_NS
#endif /* ifndef __HTTPD_REQUEST_H__ */
