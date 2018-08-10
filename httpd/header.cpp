#include "httpd/header.h"
#include "utils/string_utils.h"

BEGIN_NS(httpd)

USING_CLASS(utils, StringUtils);

static const char *const __fieldNames[] = {
    // general headers
    "Cache-Control", "Connection", "Date", "Pragma", "Trailer", "Transfer-Encoding", "Upgrade", "Via", "Warning",
    // request headers
    "Accept", "Accept-Charset", "Accept-Encoding", "Accept-Language", "Authorization", "Expect", "From", "Host",
    "If-Match", "If-Modified-Since", "If-None-Match", "If-Range", "If-Unmodified-Since", "Max-Forwards",
    "Proxy-Authorization", "Range", "Referer", "TE", "User-Agent",
    // response headers
    "Accept-Ranges", "Age", "ETag", "Location", "Proxy-Authenticate", "Retry-After", "Server", "Vary", "WWW-Authenticate",
    // entity headers
    "Allow", "Content-Encoding", "Content-Language", "Content-Length", "Content-Location", "Content-MD5", "Content-Range",
    "Content-Type", "Expires", "Last-Modified", 
};

Header::Header(const string &line) {
    string *strs[] = { &_field, &_value };
    StringUtils::split(line, ':', strs, 2);
    StringUtils::trim(_field);
    StringUtils::trim(_value);
}

Header::Header(const string &field, const string &value):
_field(field),
_value(value) {
    StringUtils::trim(_field);
    StringUtils::trim(_value);
}

Header::Header(Field field, const string &value): _value(value) {
    _field = __fieldNames[field];
    StringUtils::trim(_value);
}

void Header::set(const string &field, const string &value) {
    StringUtils::trim(_field = field);
    StringUtils::trim(_value = value);
}

void Header::set(Field field, const string &value) {
    _field = __fieldNames[field];
    StringUtils::trim(_value = value);
}

string Header::field(int filed) {
    return __fieldNames[filed];
}

END_NS
