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
static string &trim(string &str);
static string extractBetween(const string &str, const string &delim1, const string &delim2, int pos = 0);

Request::Request():
_status(PARSE_LINE),
_contentPos(0),
_contentLength(0),
_multipartPos(0),
_multipartFd(-1),
_100Continue(0),
_multipart(0),
_multipartStatus(MULTIPART_HEADERS),
_reserve(0) {
}

int Request::parseStatusLine(const char *pos, const char *last) {
    const char *p0 = NULL, *p1 = NULL, *p2 = pos;
    for (; p2 < last - 1; ++p2) {
        if (*p2 == ' ') {
            p0 ? p1 = p2 : p0 = p2;
        } else if (p2[0] == CR && p2[1] == LF) {
            _method.assign(pos, p0 - pos);
            _uri.assign(p0 + 1, p1 - p0 - 1);
            _version.assign(p1 + 1, p2 - p1 - 1);
            string::size_type p = _uri.find('?');
            if (p != string::npos) {
                _querys = urlDecode(_uri.substr(p + 1));
                _uri = _uri.substr(0, p);
            }
            _status = PARSE_HEADERS;
            return p2 - pos + 2;
        }
    }

    return 0;
}

int Request::parseHeaders(const char *pos, const char *last) {
    const char *p0 = NULL, *p1 = pos;
    int length = 0;
    for (; p1 < last - 1; ++p1) {
        if (*p1 == ':') {
            p0 = p1;
        } else if (p1[0] == CR && p1[1] == LF) {
            string field(pos, p0 - pos);
            string value(p0 + 1, p1 - p0 - 1);
            addHeader(trim(field), trim(value));
            length += p1 - pos + 2;
            if (p1 + 3 < last && p1[2] == CR && p1[3] == LF) {
                length += 2;
                _status = PARSE_CONTENT;
                break;
            }
            ++p1;
            pos = p1 + 1;
        }
    }

    return length;
}

void Request::addHeader(const string &field, const string &value) {
    _headers[field] = value;
    if (field == getFieldName(Header::Content_Length)) {
        _contentLength = atoi(value.c_str());
    } else if (field == getFieldName(Header::Expect)) {
        if (value.size() >= 3) {
            _100Continue = (value[0] == '1' && value[1] == '0' && value[2] == '0');
        }
    } else if (field == getFieldName(Header::Content_Type)) {
        _multipart = !strncasecmp(value.c_str(), "multipart/form-data", strlen("multipart/form-data"));
    }
}

int Request::parseContent(const char *pos, const char *last) {
    if (_100Continue || !_contentLength) {
        _status = PARSE_DONE;
        return 0;
    }
    int length = 0;
    if (_multipart) {
        string &type= _headers[getFieldName(Header::Content_Type)];
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

int Request::parseMultipart(const char *pos, const char *last, const string &boundary) {
    int length = 0;
    const char *p1 = pos;
    string::size_type bsize = boundary.size();
    for (; p1 < last - bsize; ++p1) {
        if (!strncmp(boundary.c_str(), p1, bsize)) {
            if (_multipartStatus == MULTIPART_HEADERS) {
                if (p1 + bsize + 2 < last && p1[bsize] == '-' && p1[bsize + 1] == '-') {
                    return length + bsize + 4;
                }
                int hlen = parseFormHeader(p1 + bsize + 2, last);
                if (!hlen) {
                    break;
                }
                p1 += hlen + bsize + 2;
                length += hlen + bsize + 2;
                pos = p1;
            } else if (_multipartStatus == MULTIPART_CONTENT) {
                length += 2 + setMultipartContent(pos, p1 - pos - 2);
                --p1;
                _multipartStatus = MULTIPART_HEADERS;
            }
        }
    }
    if (_multipartStatus == MULTIPART_CONTENT && last - pos > 0) {
        length += setMultipartContent(pos, last - pos);
    }

    return length;
}

int Request::parseFormHeader(const char *pos, const char *last) {
    const char *p0 = NULL, *p1 = pos;
    int length = 0;
    for (; p1 < last - 1; ++p1) {
        if (*p1 == ':') {
            p0 = p1;
        } else if (p1[0] == CR && p1[1] == LF) {
            length += p1 - pos + 2;
            string field(pos, p0 - pos);
            string value(p0 + 1, p1 - p0);
            _curMultipartHeader.parse(field, value);
            if (p1 + 3 < last && p1[2] == CR && p1[3] == LF) {
                _multipartStatus = MULTIPART_CONTENT;
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
    if (field == getFieldName(Header::Content_Disposition)) {
        string::size_type p = value.find("filename");
        name = extractBetween(value, "\"", "\"", p != string::npos ? p : 0);
    } else if (field == getFieldName(Header::Content_Type)) {
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

int Request::setMultipartContent(const char *pos, int length) {
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

void Request::reset(bool is100Continue) {
    _100Continue = 0;
    if (is100Continue) {
        _status = PARSE_CONTENT;
        return;
    }
    _status = PARSE_LINE;
    _uri.clear();
    _headers.clear();
    _querys.clear();
    _content.clear();
    _contentPos = 0;
}

string Request::headers() const {
    string result = _method + " " + _uri;
    if (!_querys.empty()) {
        result += "?" + _querys;
    }
    result += " " + _version + CRLF;
    for (HeaderIt it = _headers.begin(); it != _headers.end(); ++it) {
        result += it->first + ": " + it->second + CRLF;
    }
    result += CRLF;
    result += _content;

    return result;
}

const string *Request::getHeader(int field) const {
    string _field = getFieldName(field);
    HeaderIt it = _headers.find(_field);
    
    return it != _headers.end() ? &it->second : NULL;
}

string &trim(string &str) {
    const char *base = str.c_str();
    const char *p1 = base;
    const char *p2 = base + str.length() - 1;
    while(isspace(*p1)) p1++;
    while(isspace(*p2)) p2--;
    str.erase(p2 + 1 - base);
    str.erase(0, p1 - base);

    return str;
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

string extractBetween(const string &str, const string &delim1, const string &delim2, int pos) {
    string result;
    string::size_type len = delim1.length();
    string::size_type p1 = str.find(delim1, pos);
    string::size_type p2 = str.find(delim2, p1 + len);
    if (p1 != string::npos && p2 != string::npos) {
        result = str.substr(p1 + len, p2 - p1 - len);
    }

    return result;
}

} /* namespace httpd */
