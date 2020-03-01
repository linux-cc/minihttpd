#include "util/config.h"
#include "httpd/request.h"
#include "httpd/connection.h"
#include <sys/uio.h>
#include <fcntl.h>
#include <ctype.h>

namespace httpd {

static String trim(const String &str);
static String extractBetween(const String &str, const String &delim1, const String &delim2, size_t pos = 0);
static String extractBetween(const String &str, char delim1, char delim2, size_t pos = 0);

bool Request::parseHeaders() {
    if (!_ptr->_conn->recvUntil(_ptr->_headers, TWO_CRLF, false)) {
        return false;
    }
    _ptr->_uri = extractBetween(_ptr->_headers, CHAR_SP, CHAR_SP);
    size_t pos = _ptr->_headers.find("Content-Length");
    if (pos != String::npos) {
        String value = extractBetween(_ptr->_headers, STR_COLON, ONE_CRLF, pos);
        _ptr->_contentLength = atoi(value.data());
    }
    
    pos = _ptr->_headers.find("Expect");
    if (pos != String::npos) {
        String value = extractBetween(_ptr->_headers, STR_COLON, ONE_CRLF, pos);
        _ptr->_is100Continue = value[0] == '1' && value[1] == '0' && value[2] == '0';
    }
    
    pos = _ptr->_headers.find("multipart/form-data");
    _ptr->_isMultipart = pos != String::npos;
    if (_ptr->_isMultipart) {
        _ptr->_boundary = String("--").append(extractBetween(_ptr->_headers, "boundary=", ONE_CRLF, pos));
        _ptr->_content.clear();
        _ptr->_status = Parse_Form_Header;
    } else if (_ptr->_contentLength > _ptr->_content.length()) {
        _ptr->_content.resize(_ptr->_contentLength);
        _ptr->_status = Parse_Content;
    }
    
    return true;
}

String Request::getHttpVersion() const {
    size_t p1 = _ptr->_headers.find(CHAR_SP);
    size_t p2 = _ptr->_headers.find(CHAR_SP, p1 + 1);
    return extractBetween(_ptr->_headers, STR_SP, ONE_CRLF, p2);
}

String Request::getHeader(const char *field) const {
    String value = extractBetween(_ptr->_headers, field, ONE_CRLF);
    size_t pos = value.find(CHAR_COLON);
    if (pos != String::npos) {
        return trim(value.substr(pos + 1));
    }
    
    return String();
}

void Request::parseContent() {
    if (_ptr->_isMultipart) {
        while (!isCompleted()) {
            if (!parseFormHeader() || !parseFormContent()) {
                break;
            }
        }
    } else {
        char *data = (char*)_ptr->_content.data() + _ptr->_contentPos;
        size_t length = _ptr->_contentLength - _ptr->_contentPos;
        size_t n = _ptr->_conn->recv(data, length);
        _ptr->_contentPos += n;
        if (_ptr->_contentPos >= _ptr->_contentLength) {
            _ptr->_status = Parse_Finish;
        }
    }
}

bool Request::parseFormHeader() {
    if (_ptr->_status == Parse_Form_Header) {
        String buf;
        bool ok = _ptr->_conn->recvUntil(buf, TWO_CRLF, false);
        if (!ok) return false;
        _ptr->_contentPos += buf.length();
        _ptr->_status = Parse_Form_Content;
        if (buf.find("Content-Disposition") != String::npos) {
            size_t pos = buf.find("name");
            String name = extractBetween(buf, CHAR_QUOTE, CHAR_QUOTE, pos);
            if ((pos = buf.find("filename")) != String::npos) {
                name = extractBetween(buf, CHAR_QUOTE, CHAR_QUOTE, pos);
                _ptr->_formFd = open(String("/Users/linshaohua/workspace/code/minihttpd/upload/").append(name).data(), O_CREAT|O_WRONLY|O_TRUNC, 0666);
            } else {
                _ptr->_content.append("&").append(name).append("=");
            }
        }
    }
    
    return true;
}

bool Request::parseFormContent() {
    if (_ptr->_status == Parse_Form_Content) {
        String buf;
        String boundary = _ptr->_boundary;
        bool done = _ptr->_conn->recvUntil(buf, boundary.append(ONE_CRLF).data(), true);
        if (done) {
            _ptr->_status = Parse_Form_Header;
        }
        _ptr->_contentPos += buf.length();
        size_t length = buf.length() - (done ? boundary.length() + 2 : 0);
        if (_ptr->_contentPos == _ptr->_contentLength) {
            length = buf.find(_ptr->_boundary) - 2;
        }
        if (_ptr->_formFd > 0) {
            write(_ptr->_formFd, buf.data(), length);
        } else {
            _ptr->_content.append(buf.substr(0, length));
        }
        if (done || _ptr->_contentPos >= _ptr->_contentLength) {
            if (_ptr->_formFd > 0) {
                close(_ptr->_formFd);
                _ptr->_formFd = -1;
            }
            if (_ptr->_contentPos >= _ptr->_contentLength) {
                _ptr->_status = Parse_Finish;
            }
        }
        return done;
    }
    
    return true;
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
    size_t len = delim1.length();
    size_t p1 = str.find(delim1, pos);
    size_t p2 = str.find(delim2, p1 + len);
    if (p1 != String::npos && p2 != String::npos) {
        return trim(str.substr(p1 + len, p2 - p1 - len));
    }

    return String();
}

String extractBetween(const String &str, char delim1, char delim2, size_t pos) {
    size_t len = 1;
    size_t p1 = str.find(delim1, pos);
    size_t p2 = str.find(delim2, p1 + len);
    if (p1 != String::npos && p2 != String::npos) {
        return trim(str.substr(p1 + len, p2 - p1 - len));
    }
    
    return String();
}

} /* namespace httpd */
