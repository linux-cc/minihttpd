#include "util/config.h"
#include "httpd/request.h"
#include "httpd/connection.h"
#include <sys/uio.h>
#include <fcntl.h>

namespace httpd {

static String trim(const String &str);
static String extractBetween(const String &str, const String &delim1, const String &delim2, size_t pos = 0);
static String extractBetween(const String &str, char delim1, char delim2, size_t pos = 0);

Request::Request():
_contentPos(0),
_contentLength(0),
_formFd(-1),
_is100Continue(0),
_isMultipart(0),
_isParseBody(0),
_reserve(0)
{
}

bool Request::parseHeaders(Connection *conn) {
    if (!conn->recvUntil(_headers, TWO_CRLF, false)) {
        return false;
    }
    _uri = extractBetween(_headers, ' ', ' ');
    size_t pos = _headers.find("Content-Length");
    if (pos != String::npos) {
        String value = extractBetween(_headers, ":", ONE_CRLF, pos);
        _contentLength = atoi(value.data());
    }
    
    pos = _headers.find("Expect");
    if (pos != String::npos) {
        String value = extractBetween(_headers, ":", ONE_CRLF, pos);
        _is100Continue = value[0] == '1' && value[1] == '0' && value[2] == '0';
    }
    
    pos = _headers.find("multipart/form-data");
    _isMultipart = pos != String::npos;
    if (_isMultipart) {
        _boundary = extractBetween(_headers, "boundary=", ONE_CRLF, pos);
        _content.clear();
    } else if (_contentLength > _content.length()) {
        _content.resize(_contentLength);
    }
    
    return true;
}

String Request::getHeader(const char *field) const {
    String value = extractBetween(_headers, field, ONE_CRLF);
    size_t pos = value.find(':');
    if (pos != String::npos) {
        return trim(value.substr(pos + 1));
    }
    
    return String();
}

void Request::parseContent(Connection *conn) {
    if (!_isMultipart) {
        char *data = (char*)_content.data() + _contentPos;
        size_t length = _contentLength - _contentPos;
        size_t n = conn->recv(data, length);
        _contentPos += n;
        return;
    }
    String buf;
    if (!_isParseBody) {
        bool ok = conn->recvUntil(buf, TWO_CRLF, false);
        if (!ok) return;
        _contentPos += buf.length();
        if (buf.find("Content-Disposition") != String::npos) {
            size_t pos = buf.find("name");
            String name = extractBetween(buf, "\"", "\"", pos);
            if ((pos = buf.find("filename")) != String::npos) {
                name = extractBetween(buf, "\"", "\"", pos);
                _formFd = open(String("upload/").append(name).data(), O_CREAT|O_WRONLY|O_TRUNC, 0666);
            }
            _content.append("&").append(name).append("=");
        }
    }
    buf.clear();
    bool ok = conn->recvUntil(buf, ONE_CRLF, true);
    _isParseBody = !ok;
    _contentPos += buf.length();
    size_t length = buf.length() - (ok ? 2 : 0);
    if (_formFd > 0) {
        write(_formFd, buf.data(), length);
    } else {
        _content.append(buf.substr(0, length));
    }
    if ((ok || _contentPos >= _contentLength) && _formFd > 0) {
        close(_formFd);
        _formFd = -1;
    }
}

String trim(const String &str) {
    const char *base = str.data();
    const char *p1 = base;
    const char *p2 = base + str.length() - 1;
    while (isspace(*p1)) p1++;
    while (isspace(*p2)) p2--;

    return str.substr(p1 - base, p2 - p1 + 1);
}

String extractBetween(const String &str, const String &delim1, const String &delim2, size_t pos) {
    String result;
    size_t len = delim1.length();
    size_t p1 = str.find(delim1, pos);
    size_t p2 = str.find(delim2, p1 + len);
    if (p1 != String::npos && p2 != String::npos) {
        result = str.substr(p1 + len, p2 - p1 - len);
    }

    return result;
}

String extractBetween(const String &str, char delim1, char delim2, size_t pos) {
    String result;
    size_t len = 1;
    size_t p1 = str.find(delim1, pos);
    size_t p2 = str.find(delim2, p1 + len);
    if (p1 != String::npos && p2 != String::npos) {
        result = str.substr(p1 + len, p2 - p1 - len);
    }
    
    return result;
}

} /* namespace httpd */
