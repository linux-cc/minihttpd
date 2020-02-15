#include "config.h"
#include "httpd/request.h"
#include "httpd/constants.h"
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

namespace httpd {

static string urlDecode(const string &str);
static string trim(const string &str);
static string extractBetween(const string &str, const string &delim1, const string &delim2, size_t pos = 0);
static string extractBetween(const string &str, char delim1, char delim2, size_t pos = 0);

Request::Request():
_status(PARSE_HEADERS),
_contentPos(0),
_contentLength(0),
_multipartPos(0),
_multipartFd(-1) {
}

size_t Request::parse(const char *begin, const char *end) {
    const char *endline = strstr(begin, END_LINE);
    if (!endline) {
        return 0;
    }
    _headers.assign(begin, end - begin + END_LINE_LENGTH);
    _contentLength = atoi(Header::getName(Header::Content_Length).c_str());
    if (is100Continue() || !_contentLength) {
        _status = PARSE_DONE;
    }
    
    return _headers.length();
}
    
string Request::getUri() const {
    string::size_type p = _headers.find(CRLF);
    if (p != string::npos) {
        return extractBetween(_headers.substr(0, p), ' ', ' ');
    }
    
    return string();
}

string Request::getHeader(int field) const {
    string name = Header::getName(field);
    string value = extractBetween(_headers, name, CRLF);
    string::size_type p = value.find(':');
    if (p != string::npos) {
        return trim(value.substr(p + 1));
    }
    
    return string();
}

bool Request::is100Continue() const {
    string value = Header::getName(Header::Expect);
    if (value.length() >= 3) {
        return (value[0] == '1' && value[1] == '0' && value[2] == '0');
    }
    
    return false;
}
    
bool Request::isMultipart() const {
    return !strncasecmp(Header::getName(Header::Content_Type).c_str(), "multipart/form-data", strlen("multipart/form-data"));
}

size_t Request::parseContent(const char *pos, const char *last) {
    size_t length = 0;
    if (isMultipart()) {
        string type = Header::getName(Header::Content_Type);
        string::size_type eq = type.find('=');
        string boundary = "--" + trim(type.substr(eq + 1));
        length = parseMultipart(pos, last, boundary);
    } else {
        if (_contentLength + 1 > (int)_content.length()) {
            _content.resize(_contentLength);
        }
        char *data = (char*)_content.data();
        length = MIN(_contentLength - _contentPos, last - pos);
        memcpy(data + _contentPos, pos, length);
    }
    _contentPos += length;
    if (_contentPos >= _contentLength) {
        _status = PARSE_DONE;
        _content[_contentLength] = '\0';
        _content = urlDecode(_content.c_str());
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

string trim(const string &str) {
    const char *base = str.c_str();
    const char *p1 = base;
    const char *p2 = base + str.length() - 1;
    while(isspace(*p1)) p1++;
    while(isspace(*p2)) p2--;

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

string urlDecode(const string &str) {
    string decode("");
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

string extractBetween(const string &str, const string &delim1, const string &delim2, size_t pos) {
    string result;
    string::size_type len = delim1.length();
    string::size_type p1 = str.find(delim1, pos);
    string::size_type p2 = str.find(delim2, p1 + len);
    if (p1 != string::npos && p2 != string::npos) {
        result = str.substr(p1 + len, p2 - p1 - len);
    }

    return result;
}

string extractBetween(const string &str, char delim1, char delim2, size_t pos) {
    string result;
    string::size_type len = 1;
    string::size_type p1 = str.find(delim1, pos);
    string::size_type p2 = str.find(delim2, p1 + len);
    if (p1 != string::npos && p2 != string::npos) {
        result = str.substr(p1 + len, p2 - p1 - len);
    }
    
    return result;
}

} /* namespace httpd */
