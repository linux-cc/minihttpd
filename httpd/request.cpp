#include "util/config.h"
#include "httpd/request.h"
#include "httpd/constants.h"
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
_multipartPos(0),
_multipartFd(-1) {
}

bool Request::parseHeaders(const String &buf) {
    _headers = buf;
    _uri = extractBetween(_headers, ' ', ' ');
    
    size_t pos = _headers.find("Content-Length");
    if (pos != String::npos) {
        String value = extractBetween(_headers, ":", CRLF, pos);
        _contentLength = atoi(value.data());
    }
    
    pos = _headers.find("Expect");
    if (pos != String::npos) {
        String value = extractBetween(_headers, ":", CRLF, pos);
        _is100Continue = value[0] == '1' && value[1] == '0' && value[2] == '0';
    }
    
    pos = _headers.find("multipart/form-data");
    _isMultipart = pos != String::npos;
    if (_isMultipart) {
        _content.clear();
    } else if (_contentLength > _content.length()) {
        _content.resize(_contentLength);
    }
    
    return _is100Continue || !_contentLength;
}

String Request::getHeader(const char *field) const {
    String value = extractBetween(_headers, field, CRLF);
    size_t pos = value.find(':');
    if (pos != String::npos) {
        return trim(value.substr(pos + 1));
    }
    
    return String();
}

size_t Request::parseMultipart() {
    size_t length = 0;
    if (isMultipart()) {
        string type = Header::getName(Header::Content_Type);
        string::size_type eq = type.find('=');
        string boundary = "--" + trim(type.substr(eq + 1));
        length = parseMultipart(pos, last, boundary);
    } else {

    }
    _contentPos += length;
    if (_contentPos >= _contentLength) {
        _content[_contentLength] = '\0';
        _content = urlDecode(_content);
    }

    return length;
}

size_t Request::parseMultipart(const char *pos, const char *last, const string &boundary) {
    size_t length = 0;
    const char *p1 = pos;
    string::size_type bsize = boundary.size();
    for (; p1 < last - bsize; ++p1) {
        if (!strncmp(boundary.c_str(), p1, bsize)) {
            if (_status == PARSE_HEADERS) {
                if (p1 + bsize + 2 < last && p1[bsize] == '-' && p1[bsize + 1] == '-') {
                    return length + bsize + 4;
                }
                size_t hlen = parseFormHeader(p1 + bsize + 2, last);
                if (!hlen) {
                    break;
                }
                p1 += hlen + bsize + 2;
                length += hlen + bsize + 2;
                pos = p1;
            } else if (_status == PARSE_CONTENT) {
                length += 2 + setMultipartContent(pos, p1 - pos - 2);
                --p1;
            }
        }
    }
    if (_status == PARSE_CONTENT && last - pos > 0) {
        length += setMultipartContent(pos, last - pos);
    }

    return length;
}

size_t Request::parseFormHeader(const char *pos, const char *last) {
    const char *p0 = NULL, *p1 = pos;
    size_t length = 0;
    for (; p1 < last - 1; ++p1) {
        if (*p1 == ':') {
            p0 = p1;
        } else if (p1[0] == CR && p1[1] == LF) {
            length += p1 - pos + 2;
            string field(pos, p0 - pos);
            string value(p0 + 1, p1 - p0);
            _curMultipartHeader.parse(field, value);
            if (p1 + 3 < last && p1[2] == CR && p1[3] == LF) {
                _status = PARSE_CONTENT;
                length += 2;
                setMultipartName();
                if (_curMultipartHeader.hasType) {
                    string filename = "upload/" + _curMultipartHeader.name;
                    _multipartFd = open(filename.c_str(), O_CREAT|O_WRONLY|O_TRUNC, 0666);
                }
                break;
            }
            ++p1;
            pos = p1 + 1;
        }
    }

    return length;
}

void Request::MultipartHeader::parse(const string &field, const string &value) {
    hasType = false;
    if (field == Header::getName(Header::Content_Disposition)) {
        string::size_type p = value.find("filename");
        name = extractBetween(value, "\"", "\"", p != string::npos ? p : 0);
    } else if (field == Header::getName(Header::Content_Type)) {
        hasType = true;
    }
}

void Request::setMultipartName() {
    if (!_curMultipartHeader.hasType) {
        if (_multipartPos) {
            _content[_multipartPos++] = '&';
        }
        string &name = _curMultipartHeader.name;
        memcpy((char*)_content.data() + _multipartPos, name.c_str(), name.size());
        _multipartPos += name.size();
        _content[_multipartPos++] = '=';
    }
}

size_t Request::setMultipartContent(const char *pos, size_t length) {
    if (_curMultipartHeader.hasType) {
        if (_multipartFd > 0) {
            write(_multipartFd, pos, length);
        }
    } else {
        memcpy((char*)_content.data() + _multipartPos, pos, length);
        _multipartPos += length;
    }

    return length;
}

String trim(const String &str) {
    const char *base = str.c_str();
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
