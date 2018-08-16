#include "httpd/request.h"
#include "httpd/constants.h"

BEGIN_NS(httpd)

static string urlDecode(const string &str);
static string trim(const string &str);

void Request::parseStatusLine(const char *beg, const char *end) {
    string line(beg, end - beg);
    string::size_type p1 = line.find(' ');
    string::size_type p2 = line.rfind(' ');
    if (p1 != string::npos && p2 != string::npos) {
        _method = line.substr(0, p1);
        _uri = line.substr(p1 + 1, p2 - p1 - 1);
        _version = trim(line.substr(p2 + 1));
        _status = PROCESS_HEADERS;
    }
    string::size_type p = _uri.find('?');
    if (p != string::npos) {
        _querys = urlDecode(_uri.substr(p + 1));
        _uri = _uri.substr(0, p);
    }
}

void Request::addHeader(const char *beg, const char *end) {
    string header(beg, end - beg);
    string::size_type p = header.find(':');
    if (p != string::npos) {
        string field = trim(header.substr(0, p));
        string value = trim(header.substr(p + 1));
        _headers[field] = value;
        if (field == getHeaderField(Content_Length)) {
            int length = atoi(value.c_str());
            _content.resize(length);
        }
    } else {
        if (beg[0] == CR && beg[1] == LF) {
            _status = PROCESS_CONTENT;
        }
    }
}

int Request::setContent(const char *beg, const char *end) {
    int total = _content.length();
    if (has100Continue() || !total) {
        _status = PROCESS_DONE;
        return 0;
    }
    char *data = (char*)_content.data();
    int length = MIN(total - _contentIndex, end - beg);
    memcpy(data + _contentIndex, beg, length);
    _contentIndex += length;
    if (_contentIndex >= total) {
        _status = PROCESS_DONE;
        _content = urlDecode(_content);
    }
    return length;
}

void Request::reset(bool is100Continue) {
    _status = PROCESS_LINE;
    _headers.clear();
    _querys.clear();
    if (is100Continue) {
        _status = PROCESS_CONTENT;
    } else {
        _uri.clear();
        _content.clear();
    }
    _contentIndex = 0;
}

string Request::headers() const {
    string result = _method + " " + _uri;
    if (!_querys.empty()) {
        result += "?" + _querys;
    }
    result += " " + _version + CRLF;
    for (ConstIt it = _headers.begin(); it != _headers.end(); ++it) {
        result += it->first + ": " + it->second + CRLF;
    }
    result += CRLF;
    result += _content;

    return result;
}

const string *Request::getHeader(int field) const {
    string _field = getHeaderField(field);
    ConstIt it = _headers.find(_field);
    
    return it != _headers.end() ? &it->second : NULL;
}

const string *Request::getConnection() const {
    return getHeader(Connection);
}

bool Request::has100Continue() const {
    const string *value = getHeader(Expect);
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
