#include "util/config.h"
#include "httpd/response.h"
#include "httpd/request.h"
#include "httpd/connection.h"
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>

#define WEB_ROOT    "/Users/linshaohua/workspace/code/blog/html"
#define CGI_BIN     "/cgi-bin"
#define ROOT_PAGE   "/index.html"

namespace httpd {

struct Status {
    int _code;
    const char *_reason;
};

enum EStatus {
    Continue,
    OK,
    Bad_Request,
    Forbidden,
    Not_Found,
    Method_Not_Allowed,
    Internal_Server_Error,
};

static const Status __status[] = {
    { 100, "Continue" },
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
    { 405, "Method Not Allowed" },
    { 500, "Internal Server Error" },
};

static String itoa(int i) {
    char temp[16] = { 0 };
    sprintf(temp, "%d", i);
    return temp;
}

static String getGMTTime(time_t t) {
    char buf[32] = { 0 };
    time_t _t = t ? t : time(NULL);
    struct tm *tm = gmtime(&_t);
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S GMT", tm);
   
    return buf;
}

static void eraseBetween(String &str, const char *delim1, const char *delim2) {
    size_t p1 = str.find(delim1);
    if (p1 != String::npos) {
        size_t p2 = str.find(delim2, p1 + strlen(delim1));
        if (p2 != String::npos) {
            str.erase(p1, p2 + strlen(delim2) - p1);
        }
    }
}

void Response::parseRequest(const Request *req) {
    if (_headers.empty()) {
        if (!req->isGet() && !req->isPost()) {
            setStatusLine(Method_Not_Allowed, req->getHttpVersion());
        } else if (req->is100Continue()) {
            setStatusLine(Continue, req->getHttpVersion());
        } else {
            String file = String(WEB_ROOT).append(parseUri(req->getUri()));
            if (file.empty()) {
                setStatusLine(Bad_Request, req->getHttpVersion());
            } else {
                setStatusLine(parseFile(file), req->getHttpVersion());
            }
        }
        setCommonHeaders(req);
    }
}

void Response::setStatusLine(int status, const String &version) {
    _code = status;
    String line;
    line.append(version).append(CHAR_SP);
    line.append(itoa(__status[status]._code)).append(CHAR_SP);
    line.append(__status[status]._reason).append(ONE_CRLF);
    _headers.insert(0, line);
}

String Response::parseUri(const String &uri) {
    if (uri[0] == '/') {
        return uri == "/" ? ROOT_PAGE : uri;
    }
    size_t p = uri.find("//");
    if (p != String::npos) {
        p = uri.find('/', p + 2);
        if (p != String::npos) {
            return uri.substr(p);
        }
    } else {
        p = uri.find('/');
        if (p != String::npos) {
            return uri.substr(p);
        }
    }
    return "";
}

int Response::parseFile(const String &file) {
    struct stat st;
    if (stat(file.data(), &st)) {
        return Not_Found;
    }
    _cgiBin = !strncasecmp(file.data(), CGI_BIN, strlen(CGI_BIN));
    int permit = S_IRUSR | S_IRGRP | S_IROTH | (_cgiBin ? S_IXUSR : 0);
    if (!(st.st_mode & permit)) {
        return Forbidden;
    }
    _fd = open(file.data(), O_RDONLY | O_NONBLOCK);
    if (_fd < 0) {
        return Internal_Server_Error;
    }
    
    _fileLength = st.st_size;
    setContentType(file);
    _headers.append("Content-Length: ").append(itoa(_fileLength)).append(ONE_CRLF);
    _headers.append("Last-Modified: ").append(getGMTTime(STAT_MTIME(st))).append(ONE_CRLF);
    
    return OK;
}

void Response::setContentType(const String &file) {
    size_t p = file.find('.');
    String subfix = "html";
    if (p != String::npos) {
        subfix = file.substr(p + 1);
    }
    _headers.append("Content-Type: ");
    if (!strncasecmp(subfix.data(), "htm", 3)) {
        _headers.append("text/html");
    } else if (!strncasecmp(subfix.data(), "js", 2)) {
        _headers.append("application/javascript");
    } else if (!strncasecmp(subfix.data(), "jpg", 3) || !strncasecmp(subfix.data(), "jpeg", 4)) {
        _headers.append("image/jpeg");
    } else if (!strncasecmp(subfix.data(), "png", 3)) {
        _headers.append("image/png)");
    } else if (!strncasecmp(subfix.data(), "gif", 3)) {
        _headers.append("image/gif)");
    } else {
        _headers.append("text/").append(subfix);
    }
    _headers.append("; charset=UTF-8").append(ONE_CRLF);
}

void Response::setCommonHeaders(const Request *req) {
    String value = req->getHeader("Connection");
    if (!value.empty()) {
        _headers.append("Connection: ").append(value).append(ONE_CRLF);
        _connClose = value == "close";
    }    
    if (_code == OK) {
        value = req->getHeader("Accept-Encoding");
        if (value.find("gzip") != String::npos) {
            _headers.append("Transfer-Encoding: chunked").append(ONE_CRLF);
            _headers.append("Content-Encoding: gzip").append(ONE_CRLF);
            eraseBetween(_headers, "Content-Length", ONE_CRLF);
            _acceptGz = 1;
        }
    }
    _headers.append("Server: minihttpd/1.1.01").append(ONE_CRLF);
    _headers.append("Date: ").append(getGMTTime(0)).append(TWO_CRLF);
}

bool Response::sendCompleted() const {
    if (_headPos >= _headers.length()) {
        if (_code == Continue || _gzEof) {
            return true;
        }
        
        return _filePos >= _fileLength;
    }
    
    return false;
}

bool Response::sendResponse() {
    if (_headPos < _headers.length()) {
        ssize_t n = _conn->send(_headers);
        if (n < 0) return false;
        _headPos += n;
    }
    
    if (_code != Continue) {
        GZip gzip(this);
        if (!_acceptGz || !gzip.init()) {
            return sendFile() < 0;
        }
        gzip.zip();
    }
    
    return true;
}

ssize_t Response::sendFile() {
    if (_fd > 0 && _filePos < _fileLength) {
        off_t n = _fileLength - _filePos;
#ifdef __linux__
        sszie_t ret = sendfile(_conn->fd(), _fd, &_filePos, _fileLength - _filePos);
#else
        int ret = sendfile(_fd, _conn->fd(), _filePos, &n, NULL, 0);
#endif
        if (ret < 0 && errno != EAGAIN) {
            return ret;
        }
        _filePos += n;
        return n;
    }
    
    return 0;
}

int Response::gzfill(void *buf, int len) {
    return read(_fd, buf, len);
}

bool Response::gzflush(const void *buf, int len, bool eof) {
    static char tailer[] = { CHAR_CR, CHAR_LF, '0', CHAR_CR, CHAR_LF, CHAR_CR, CHAR_LF, };
    char header[16] = { 0 };
    int hlen = sprintf(header, "%x%s", len, ONE_CRLF);
    struct iovec iov[3];
    
    iov[0].iov_base = header;
    iov[0].iov_len = hlen;
    iov[1].iov_base = (void*)buf;
    iov[1].iov_len = len;
    iov[2].iov_base = tailer;
    iov[2].iov_len = eof ? 7 : 2;
    _gzEof = eof;
    
    return _conn->send(iov, 3) == hlen + len + (eof ? 7 : 2);
}

} /* namespace httpd */
