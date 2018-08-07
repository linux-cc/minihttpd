#ifndef __HTTPD_STATUS_LINE_H__
#define __HTTPD_STATUS_LINE_H__

#include "config.h"
#include <string>

BEGIN_NS(httpd)

using std::string;

class RequestStatusLine {
public:
    RequestStatusLine(const string &line = "") {
        parse(line);
    }
    void parse(const string &line);
    void method(const string &method) {
        _method = method;
    }
    void uri(const string &uri) {
        _uri = uri;
    }
    void version(const string &version) {
        _version = version;
    }
private:
    string _method;
    string _uri;
    string _version;
};

class ResponseStatusLine {
public:
    ResponseStatusLine(const string &line = "") {
        parse(line);
    }
    void parse(const string &line);
    void version(const string &version) {
        _version = version;
    }
    void status(const string &status) {
        _status = status;
    }
    void cause(const string &cause) {
        _cause = cause;
    }
private:
    string _version;
    string _status;
    string _cause;
};
END_NS
#endif /* ifndef __HTTPD_STATUS_LINE_H__ */
