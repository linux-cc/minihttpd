#include "httpd/request.h"
#include "httpd/connection.h"

BEGIN_NS(httpd)

static string urlDecode(const string &str);
static string trim(const string &str);

int Request::parseStatusLine(const char *pos) {
    const char *end = strstr(pos, CRLF);
    if (end) {
        string line(pos, end - pos);
        string::size_type p1 = line.find(' ');
        string::size_type p2 = line.rfind(' ');
        if (p1 != string::npos && p2 != string::npos) {
            _method = line.substr(0, p1);
            _uri = line.substr(p1 + 1, p2 - p1 - 1);
            _version = trim(line.substr(p2 + 1));
            _status = PARSE_HEADERS;
        }
        string::size_type p = _uri.find('?');
        if (p != string::npos) {
            _querys = urlDecode(_uri.substr(p + 1));
            _uri = _uri.substr(0, p);
        }
        return end - pos + 2;
    }

    return 0;
}

int Request::parseHeaders(const char *pos) {
    const char *end;
    int length = 0;
    while ((end = strstr(pos, CRLF))) {
        length += end - pos + 2;
        string header(pos, end - pos);
        string::size_type p = header.find(':');
        if (p != string::npos) {
            string field = trim(header.substr(0, p));
            string value = trim(header.substr(p + 1));
            _headers[field] = value;
            if (field == getFieldName(Header::Content_Length)) {
                int length = atoi(value.c_str());
                _content.resize(length);
            }
        } else {
            if (pos[0] == CR && pos[1] == LF) {
                _status = PARSE_CONTENT;
                break;
            }
        }
        pos = end + 2;
    }

    return length;
}

int Request::parseContent(const char *pos, const char *last) {
    int total = _content.length();
    if (has100Continue() || !total) {
        _status = PARSE_DONE;
        return 0;
    }
    char *data = (char*)_content.data();
    int length = MIN(total - _contentPos, last - pos);
    memcpy(data + _contentPos, pos, length);
    _contentPos += length;
    if (_contentPos >= total) {
        _status = PARSE_DONE;
        _content = urlDecode(_content);
    }

    return length;
}

void Request::reset(bool is100Continue) {
    if (is100Continue) {
        _status = PARSE_CONTENT;
        _headers.erase(getFieldName(Header::Expect));
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

bool Request::has100Continue() const {
    const string *value = getHeader(Header::Expect);
    if (value && value->size() >= 3) {
        const string &v = *value;
        return v[0] == '1' && v[1] == '0' && v[2] == '0';
    }

    return false;
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

END_NS
