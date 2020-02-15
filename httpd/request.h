#ifndef __HTTPD_REQUEST_H__
#define __HTTPD_REQUEST_H__

#include <stdint.h>
#include <strings.h>
#include <string>

namespace httpd {

using std::string;
class Connection;

class Request {
public:
    Request();
    size_t parse(const char *begin, const char *end);
    string getHeader(int field) const;
    bool is100Continue() const;
    bool isMultipart() const;
    
    string getUri() const;
    
    const string &headers() const  {
        return _headers;
    }
    bool isGet() const {
        return !strncasecmp(_headers.c_str(), "GET", 3);
    }
    bool isPost() const {
        return !strncasecmp(_headers.c_str(), "POST", 4);
    }
    bool isComplete() const {
        return _status == PARSE_DONE;
    }
    size_t parseContent(const char *pos, const char *last);
    
private:
    enum Status {
        PARSE_HEADERS,
        PARSE_CONTENT,
        PARSE_DONE,
    };
    struct MultipartHeader {
        void parse(const string &field, const string &value);

        string name;
        string value;
        bool hasType;
    };
    size_t parseMultipart(const char *pos, const char *last, const string &boundary);
    size_t parseFormHeader(const char *pos, const char *last);
    void setMultipartName();
    size_t setMultipartContent(const char *pos, size_t length);
    int parseFormBody(const char *pos, const char *last);
    void addHeader(const string &field, const string &value);

    string _headers;
    Status _status;
    string _content;
    MultipartHeader _curMultipartHeader;
    int _contentPos;
    int _contentLength;
    int _multipartPos;
    int _multipartFd;

};

} /* namespace httpd */
#endif /* ifndef __HTTPD_REQUEST_H__ */
