#include "httpd/response.h"
#include "httpd/request.h"
#include "httpd/constants.h"
#include "httpd/connection.h"
#include <sys/socket.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define WEB_ROOT    "./htdocs/html"
#define CGI_BIN     "/cgi-bin"
#define ROOT_PAGE   "/index.html"

BEGIN_NS(httpd)

static string itoa(int i) {
    char temp[16] = { 0 };
    sprintf(temp, "%d", i);
    return temp;
}

static string getGMTTime(time_t t) {
    char buf[32] = { 0 };
    time_t _t = t ? t : time(NULL);
    struct tm *tm = gmtime(&_t);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
   
    return buf;
}

void Response::parseRequest(const Request &request) {
    if (!request.isGet() && !request.isPost()) {
        setStatusLine(ResponseStatus::Not_Implemented, request.version());
    } else if (!request.isHttp11()) {
        setStatusLine(ResponseStatus::HTTP_Version_Not_Supported, request.version());
    } else if (request.has100Continue()) {
        setStatusLine(ResponseStatus::Continue, request.version());
    } else {
        string file = WEB_ROOT + parseUri(request.uri());
        if (file.empty()) {
            setStatusLine(ResponseStatus::Bad_Request, request.version());
        } else {
            int status = parseFile(file);
            setStatusLine(status, request.version());
        }
    }
    setCommonHeaders(request);
    setHeadersStr();
    _status = SEND_HEADERS;
}

void Response::setCommonHeaders(const Request &request) {
    const string *value = request.getHeader(Header::Connection);
    if (value) {
        _headers[Header::Connection] = *value;
        _connClose = !strncasecmp(value->c_str(), "close", 5);
    }    
    value = request.getHeader(Header::Accept_Encoding);
    if (value && value->find("gzip") != string::npos) {
        _headers[Header::Transfer_Encoding] = "chunked";
        _headers[Header::Content_Encoding] = "gzip";
        _headers.erase(Header::Content_Length);
        _hasGzip = 1;
    }
    _headers[Header::Server] = "myframe/httpd/1.1.01";
    _headers[Header::Date] = getGMTTime(0);
}

bool Response::sendHeaders(Connection *conn) {
    const char *data = _headersStr.data() + _headersPos;
    int length = _headersStr.length();
    int left = length - _headersPos;
    int n = conn->send(data, left);
    if (n < 0) {
        return false;
    }
    _headersPos += n;
    if (_headersPos >= length) {
        _status = _fd > 0 ? SEND_CONTENT : SEND_DONE;
    }
    return true;
}

bool Response::sendContentOriginal(Connection *conn) {
    if (_fd < 0 || _fileLength == 0) {
        _status = SEND_DONE;
        return true;
    }
    off_t n = _fileLength - _filePos;
#ifdef __linux__
    n = sendfile(*conn, _fd, &_filePos, n);
    if (n < 0) {
        return errno != EAGAIN ? false : true;
    }
#else
    if (sendfile(_fd, *conn, _filePos, &n, NULL, 0)) {
        return errno != EAGAIN ? false : true;
    }
#endif
    _filePos += n + 1;
    if (_filePos >= _fileLength) {
       _status = SEND_DONE;
    }
    return true;
}

bool Response::sendContentChunked(Connection *conn) {
    return false;
}

void Response::setStatusLine(int status, const string &version) {
    _version = version;
    _code = itoa(status);
    _reason = getStatusReason(status);
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
        return ResponseStatus::Not_Found;
    }
    _cgiBin = !strncasecmp(file.c_str(), CGI_BIN, strlen(CGI_BIN));
    int permit = S_IRUSR | S_IRGRP | S_IROTH | (_cgiBin ? S_IXUSR : 0);
    if (!(st.st_mode & permit)) {
        return ResponseStatus::Forbidden;
    }
    _fd = open(file.c_str(), O_RDONLY | O_NONBLOCK);
    if (_fd < 0) {
        return ResponseStatus::Internal_Server_Error;
    }
    setContentInfo(file, st);

    return ResponseStatus::OK;
}

void Response::setContentInfo(const string &file, const struct stat &st) {
    setContentType(file);
    _fileLength = st.st_size;
    _headers[Header::Content_Length] = itoa(st.st_size);
    _headers[Header::Last_Modified] = getGMTTime(STAT_MTIME(st));
}

void Response::setContentType(const string &file) {
    string::size_type p = file.rfind('.');
    string subfix = "html";
    if (p != string::npos) {
        subfix = file.substr(p + 1);
    }
    string &value = _headers[Header::Content_Type];
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

void Response::setHeadersStr(){
    _headersStr = _version + ' ' + _code + ' ' + _reason + CRLF;
    for (HeaderIt it = _headers.begin(); it != _headers.end(); ++it) {
        _headersStr += getFieldName(it->first) + ": " + it->second + CRLF;
    }
    _headersStr += CRLF;
}

void Response::reset() {
    if (_fd > 0) {
        close(_fd);
    }
    _fd = -1;
    _headers.clear();
    _status = PARSE_REQUEST;
    _headersStr.clear();
    _headersPos = 0;
    _filePos = 0;
    _fileLength = 0;
    _cgiBin = 0;
    _connClose = 0;
    _hasGzip = 0;
    _reserve = 0;
}

END_NS
