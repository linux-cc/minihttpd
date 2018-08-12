#include "httpd/response.h"
#include "httpd/request.h"
#include "utils/string_utils.h"
#include <sys/stat.h>
#include <fcntl.h>

#define WEB_ROOT    "./htdocs/html"
#define CGI_BIN     "/cgi-bin"
#define ROOT_PAGE   "/index.html"
#define CRLF        "\r\n"

BEGIN_NS(httpd)

USING_CLASS(utils, StringUtils);

#define put_reason(code)\
do {\
    string reason = #code;\
    string::size_type p;\
    while ((p = reason.find('_')) != string::npos) {\
        reason[p] = ' ';\
    }\
    _reasons[code] = reason;\
}while (0)

map<int, string> Response::_reasons;

void Response::initStatus() {
    if (!_reasons.empty()) {
        return;
    }
    // Informational 1xx
    put_reason(Continue);
    put_reason(Switching_Protocols);
    // Successful 2xx
    put_reason(OK);
    put_reason(Created);
    put_reason(Accepted);
    put_reason(Non_Authoritative_Information);
    put_reason(No_Content);
    put_reason(Reset_Content);
    put_reason(Partial_Content);
    // Redirection 3xx
    put_reason(Multiple_Choices);
    put_reason(Moved_Permanently);
    put_reason(Found);
    put_reason(See_Other);
    put_reason(Not_Modified);
    put_reason(Use_Proxy);
    put_reason(Unused);
    put_reason(Temporary_Redirect);
    // Client Error 4xx
    put_reason(Bad_Request);
    put_reason(Unauthorized);
    put_reason(Payment_Required);
    put_reason(Forbidden);
    put_reason(Not_Found);
    put_reason(Method_Not_Allowed);
    put_reason(Not_Acceptable);
    put_reason(Proxy_Authentication_Required);
    put_reason(Request_Timeout);
    put_reason(Conflict);
    put_reason(Gone);
    put_reason(Length_Required);
    put_reason(Precondition_Failed);
    put_reason(Request_Entity_Too_Large);
    put_reason(Request_URI_Too_Long);
    put_reason(Unsupported_Media_Type);
    put_reason(Requested_Range_Not_Satisfiable);
    put_reason(Expectation_Failed);
    // Server Error 5xx
    put_reason(Internal_Server_Error);
    put_reason(Not_Implemented);
    put_reason(Bad_Gateway);
    put_reason(Service_Unavailable);
    put_reason(Gateway_Timeout);
    put_reason(HTTP_Version_Not_Supported);
}

void Response::parseRequest(const Request &request) {
    if (!request.isGet() && !request.isPost()) {
        setStatusLine(Not_Implemented, request.version());
    } else if (!request.isHttp11()) {
        setStatusLine(HTTP_Version_Not_Supported, request.version());
    } else {
        string file = WEB_ROOT + parseUri(request.uri());
        if (file.empty()) {
            setStatusLine(Bad_Request, request.version());
        } else {
            int code = parseFile(file);
            setStatusLine(code, request.version());
            setContentType(file);
            setContentLength(_contentLength);
        }
    }
    _headers += string("Connection: keep-alive") + CRLF;
    _headers += string("Server: myframe httpd 1.0") + CRLF;
    _headers += CRLF;
}

int Response::parseFile(const string &file) {
    struct stat st;
    if (stat(file.c_str(), &st)) {
        return Not_Found;
    }
    _cgiBin = !strncasecmp(file.c_str(), CGI_BIN, strlen(CGI_BIN));
    int permit = S_IRUSR | S_IRGRP | S_IROTH | (_cgiBin ? S_IXUSR : 0);
    if (!(st.st_mode & permit)) {
        return Forbidden;
    }
    _fd = open(file.c_str(), O_RDONLY);
    if (_fd < 0) {
        return Internal_Server_Error;
    }
    _contentLength = st.st_size;

    return OK;
}

string Response::parseUri(const string &uri) {
    if (uri[0] == '/') {
        return uri == "/" ? ROOT_PAGE : uri;
    }
    string::size_type p = uri.find("//");
    if (p != string::npos) {
        p = uri.find('/', p + 2);
        if (p != string::npos) {
            return uri.substr(p);
        }
    } else {
        p = uri.find('/');
        if (p != string::npos) {
            return uri.substr(p);
        }
    }
    return "";
}

void Response::setStatusLine(int status, const string &version) {
    initStatus();
    _headers = version + ' ';
    _headers += StringUtils::itoa(status) + ' ';
    _headers += _reasons[status] + CRLF;
}

void Response::setContentType(const string &file) {
    string::size_type p = file.rfind('.');
    string subfix = "html";
    if (p != string::npos) {
        subfix = file.substr(p + 1);
    }
    _headers += Header::field(Header::Content_Type) + ": ";
    if (!strncasecmp(subfix.c_str(), "htm", 3)) {
        _headers += "text/html";
    } else if (!strncasecmp(subfix.c_str(), "js", 2)) {
        _headers += "application/javascript";
    } else if (!strncasecmp(subfix.c_str(), "jpg", 3) || !strncasecmp(subfix.c_str(), "jpeg", 4)) {
        _headers += "image/jpeg";
    } else if (!strncasecmp(subfix.c_str(), "png", 3)) {
        _headers += "image/png";
    } else if (!strncasecmp(subfix.c_str(), "gif", 3)) {
        _headers += "image/gif";
    } else {
        _headers += "text/" + subfix;
    }
    _headers += "; charset=utf-8";
    _headers += CRLF;
}

void Response::setContentLength(off_t length) {
    _headers += Header::field(Header::Content_Length) + ": ";
    _headers += StringUtils::itoa(length) + CRLF;
}

END_NS
