#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "config.h"
#include "httpd/header.h"
#include "httpd/status_line.h"
#include <vector>

BEGIN_NS(httpd)

using std::vector;

class Request{
public:
    Request() {}
    void parseStatusLine(const string &line) {
        _statusLine.parse(line);
    }
    bool isGet() const {
        return _statusLine.isGet();
    }
    bool isPost() const {
        return _statusLine.isPost();
    }
private:
    RequestStatusLine _statusLine;
    vector<Header> _headrs;
};

END_NS
#endif /* ifndef __HTTPD_REQUEST_H__ */
