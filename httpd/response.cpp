#include "httpd/response.h"
#include "httpd/request.h"
#include "httpd/constants.h"
#include <sys/stat.h>
#include <fcntl.h>

#define WEB_ROOT    "./htdocs/html"
#define CGI_BIN     "/cgi-bin"
#define ROOT_PAGE   "/index.html"

BEGIN_NS(httpd)

static string itoa(int i) {
    char temp[16] = { 0 };
    sprintf(temp, "%d", i);
    return temp;
}

inline void Response::setContentLength(off_t length) {
    _contentLength = length;
    _headers[getHeaderField(Content_Length)] = itoa(length);
}

inline void Response::setServer() {
    _headers[getHeaderField(Server)] = "myframe/httpd/1.0.01";
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
            int status = parseFile(file);
            setStatusLine(status, request.version());
        }
    }
    setConnection(request);
    setServer();
    setHeaders();
    _status = SEND_HEADERS;
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
    setContentType(file);
    setContentLength(st.st_size);

    return OK;
}

void Response::setStatusLine(int status, const string &version) {
    _version = version;
    _code = itoa(status);
    _reason = getStatusReason(status);
}

void Response::setContentType(const string &file) {
    string::size_type p = file.rfind('.');
    string subfix = "html";
    if (p != string::npos) {
        subfix = file.substr(p + 1);
    }
    string field = getHeaderField(Content_Type);
    string &value = _headers[field];
    if (!strncasecmp(subfix.c_str(), "htm", 3)) {
        value = "text/html";
    } else if (!strncasecmp(subfix.c_str(), "js", 2)) {
        value = "application/javascript";
    } else if (!strncasecmp(subfix.c_str(), "jpg", 3) || !strncasecmp(subfix.c_str(), "jpeg", 4)) {
        value = "image/jpeg";
    } else if (!strncasecmp(subfix.c_str(), "png", 3)) {
        value = "image/png";
    } else if (!strncasecmp(subfix.c_str(), "gif", 3)) {
        value = "image/gif";
    } else {
        value = "text/" + subfix;
    }
    value += "; charset=UTF-8";
}

void Response::setConnection(const Request &request) {
    string field = getHeaderField(Connection);
    const string *value = request.getConnection();
    if (value) {
        _headers[field] = *value;
    }    
}

bool Response::connectionClose() const {
    ConstIt it = _headers.find(getHeaderField(Connection));
    if (it != _headers.end() && !strncasecmp(it->second.c_str(), "close", 5)) {
        return true;
    }

    return false;
}

void Response::setHeaders(){
    _headersStr = _version + ' ' + _code + ' ' + _reason + CRLF;
    for (ConstIt it = _headers.begin(); it != _headers.end(); ++it) {
        _headersStr += it->first + ": " + it->second + CRLF;
    }
    _headersStr += CRLF;
}

void Response::addHeadersPos(int pos) {
    _headersPos += pos;
    if (_headersPos >= (int)_headersStr.length()) {
        _status = _fd > 0 ? SEND_CONTENT : SEND_DONE;
    }
}

void Response::reset() {
    _cgiBin = false;
    _fd = -1;
    _headers.clear();
    _status = PARSE_REQUEST;
    _headersStr.clear();
    _headersPos = 0;
    _filePos = 0;
    _contentLength = 0;
}

END_NS
