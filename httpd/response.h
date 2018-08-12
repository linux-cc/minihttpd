#ifndef __HTTPD_RESPONSE_H__
#define __HTTPD_RESPONSE_H__

#include "config.h"
#include <unistd.h>
#include <string>
#include <map>

BEGIN_NS(httpd)

using std::string;
using std::map;
class Request;

class Response {
public:
    enum ReusultCode {
        // Informational 1xx
        Continue = 100, Switching_Protocols, 
        // Successful 2xx
        OK = 200, Created, Accepted, Non_Authoritative_Information, No_Content, Reset_Content, Partial_Content,
        // Redirection 3xx
        Multiple_Choices = 300, Moved_Permanently, Found, See_Other, Not_Modified, Use_Proxy, Unused, Temporary_Redirect,
        // Client Error 4xx
        Bad_Request = 400, Unauthorized, Payment_Required, Forbidden, Not_Found, Method_Not_Allowed, Not_Acceptable,
        Proxy_Authentication_Required, Request_Timeout, Conflict, Gone, Length_Required, Precondition_Failed,
        Request_Entity_Too_Large, Request_URI_Too_Long, Unsupported_Media_Type, Requested_Range_Not_Satisfiable,
        Expectation_Failed,
        // Server Error 5xx
        Internal_Server_Error = 500, Not_Implemented, Bad_Gateway, Service_Unavailable, Gateway_Timeout,
        HTTP_Version_Not_Supported, 
    };
    Response(): _cgiBin(false), _fd(-1), _contentLength(0) {}
    ~Response() { if (_fd > 0) close(_fd); }
    void parseRequest(const Request &request);
    const string &headers() {
        return _headers;
    }
    int contentLength() const {
        return _contentLength;
    }
    int fd() const {
        return _fd;
    }

private:
    int parseFile(const string &file);
    void setStatusLine(int status, const string &version);
    void initStatus();
    string parseUri(const string &uri);
    void setContentType(const string &file);
    void setContentLength(off_t length);

    bool _cgiBin;
    int _fd;
    off_t _contentLength;
    string _headers;
    static map<int, string> _reasons;
};

END_NS
#endif /* ifndef __HTTPD_RESPONSE_H__ */
