#ifndef __HTTPD_STATUS_LINE_H__
#define __HTTPD_STATUS_LINE_H__

#include "config.h"
#include "utils/string_utils.h"

BEGIN_NS(httpd)

using std::string;
USING_CLASS(utils, StringUtils);

class RequestStatusLine {
public:
    void parse(const string &line) {
        string *strs[] = { &_method, &_uri, &_version };
        StringUtils::split(line, ' ', strs, 3);
    }
    void method(const string &method) {
        _method = method;
    }
    void uri(const string &uri) {
        _uri = uri;
    }
    void version(const string &version) {
        _version = version;
    }
    bool isValid() const {
        return (_method == "GET" || _method == "POST")
            && (_version == "HTTP/1.0" || _version == "HTTP/1.1");
    }
    bool isGet() const {
        return _method == "GET";
    }
    bool isPost() const {
        return _method == "POST";
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
    void parse(const string &line) {
        string *strs[] = { &_version, &_status, &_cause };
        StringUtils::split(line, ' ', strs, 3);
    }
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
