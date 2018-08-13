#include "httpd/request.h"
#include "httpd/constants.h"

BEGIN_NS(httpd)

static string trim(const string &str) {
    const char *base = str.c_str();
    const char *p1 = base;
    const char *p2 = base + str.length() - 1;
    while(isspace(*p1)) p1++;
    while(isspace(*p2)) p2--;

    return str.substr(p1 - base, p2 - p1 + 1);
}

static char hexToChar(char hex) {
    if (hex >= 'A' && hex <= 'F') {
        return hex - 'A';
    }
    if (hex >= 'a' && hex <= 'f') {
        return hex - 'a';
    }

    return hex - '0';
}

static string urlDecode(const string &str) {
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

bool Request::addHeader(const string &line) {
    string::size_type p = line.find(':');
    if (p != string::npos) {
        string field = trim(line.substr(0, p));
        string value = trim(line.substr(p + 1));
        _headers[field] = value;
        if (field == getHeaderField(Content_Length)) {
            int length = atoi(value.c_str());
            _content.resize(length);
        }
        return true;
    }
    return false;
}

void Request::parseStatusLine(const string &line) {
    string::size_type p1 = line.find(' ');
    string::size_type p2 = line.rfind(' ');
    if (p1 != string::npos && p2 != string::npos) {
        _method = line.substr(0, p1);
        _uri = line.substr(p1 + 1, p2 - p1 - 1);
        _version = trim(line.substr(p2 + 1));
    }
    string::size_type p = _uri.find('?');
    if (p != string::npos) {
        _querys = urlDecode(_uri.substr(p + 1));
        _uri = _uri.substr(0, p);
    }
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

const string *Request::getConnection() const {
    string field = getHeaderField(Connection);
    ConstIt it = _headers.find(field);
    
    return it != _headers.end() ? &it->second : NULL;
}

END_NS
