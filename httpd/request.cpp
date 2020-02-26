#include "util/config.h"
#include "httpd/request.h"
#include "httpd/connection.h"
#include <sys/uio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

namespace httpd {

static String urlDecode(const String &str);
static String trim(const String &str);
static String extractBetween(const String &str, const String &delim1, const String &delim2, size_t pos = 0);
static String extractBetween(const String &str, char delim1, char delim2, size_t pos = 0);

Request::Request():
_contentPos(0),
_contentLength(0),
_formFd(-1)
{
}

bool Request::parseHeaders(Connection *conn) {
    if (!conn->recvUntil(_headers, TWO_CRLF)) {
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
    String buf, name;
    String boundary = extractBetween(_headers, "boundary=", ONE_CRLF).append(ONE_CRLF);
    while (_contentPos < _contentLength && conn->recvLine(buf)) {
        _contentPos += buf.length();
        if (buf.find("Content-Disposition") != String::npos) {
            size_t pos = buf.find("name");
            name = extractBetween(buf, "\"", "\"", pos);
            if ((pos = buf.find("filename")) != String::npos) {
                name = extractBetween(buf, "\"", "\"", pos);
                _formFd = open(String("upload/").append(name).data(), O_CREAT|O_WRONLY|O_TRUNC, 0666);
            }
        }
        if (buf == ONE_CRLF) {
            buf.clear();
            bool ok = conn->recvUntil(buf, boundary.data());
            _contentPos += buf.length();
            if (ok) {
                if (_formFd > 0) {
                    if (!_buffer.enqueue(buf.data(), buf.length())) {
                        struct iovec iov[2];
                        int niov = _buffer.getReadIov(iov);
                        ssize_t n = writev(_formFd, iov, niov);
                        _buffer.setReadPos(n);
                        _buffer.enqueue(buf.data(), buf.length());
                    }
                } else {
                    size_t len = buf.length() - boundary.length() - 2;
                    _content.append(name).append("=").append(buf.substr(0, len)).append("&");
                }
            }
        }
    }
    if (_contentPos >= _contentLength) {
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

char hexToChar(char hex) {
    if (hex >= 'A' && hex <= 'F') {
        return hex - 'A';
    }
    if (hex >= 'a' && hex <= 'f') {
        return hex - 'a';
    }

    return hex - '0';
}

String urlDecode(const String &str) {
    String decode("");
    size_t length = str.length();
    for (size_t i = 0; i < length; ++i) {
        if (str[i] == '+') {
            decode += ' ';
        } else if (str[i] == '%')
        {
            uint8_t high = hexToChar(str[++i]);
            uint8_t low = hexToChar(str[++i]);
            decode += (high << 4) | low;
        } else {
            decode += str[i];
        }
    }

    return decode;
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
