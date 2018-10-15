#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include "config.h"
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
    Request();
    int parseStatusLine(const char *pos, const char *last);
    int parseHeaders(const char *pos, const char *last);
    int parseContent(const char *pos, const char *last);
    void reset(bool is100Continue);
    string headers() const;
    const string *getHeader(int field) const;

    bool has100Continue() const {
        return _100Continue;
    }
    const string &content() const {
        return _content;
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
    enum {
        MULTIPART_HEADERS,
        MULTIPART_CONTENT,
    };
    struct MultipartHeader {
        void parse(const string &field, const string &value);

        string name;
        string value;
        bool hasType;
    };
    int parseMultipart(const char *pos, const char *last, const string &boundary);
    int parseFormdata(const char *pos, const char *last);
    int parseFormHeader(const char *pos, const char *last);
    void setMultipartName();
    int setMultipartContent(const char *pos, int length);
    int parseFormBody(const char *pos, const char *last);
    void addHeader(const string &field, const string &value);

    typedef map<string, string>::const_iterator HeaderIt;
    string _method;
    string _uri;
    string _version;
    map<string, string> _headers;
    string _querys;
    string _content;
    Status _status;
    MultipartHeader _curMultipartHeader;
    int _contentPos;
    int _contentLength;
    int _multipartPos;
    int _multipartFd;
    uint8_t _100Continue: 1;
    uint8_t _multipart: 1;
    uint8_t _multipartStatus: 1;
    uint8_t _reserve: 5;
};

END_NS
#endif /* ifndef __HTTPD_REQUEST_H__ */
