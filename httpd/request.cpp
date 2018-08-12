#include "httpd/request.h"
#include "utils/string_utils.h"

BEGIN_NS(httpd)

USING_CLASS(utils, StringUtils);

void Request::addHeader(const Header &header) {
    const string &field = header.field();
    _headrs[field] = header.value();
    if (field == Header::field(Header::Content_Length)) {
        int length = atoi(header.value().c_str());
        _content.resize(length);
    }
}

void Request::parseStatusLine(const string &line) {
    string *strs[] = { &_statusLine.method, &_statusLine.uri, &_statusLine.version };
    StringUtils::split(line, ' ', strs, 3);
    for (int i = 0; i < 3; ++i) {
        StringUtils::trim(*strs[i]);
    }
    _LOG_("read status line method: %s, uri: %s, version: %s\n",
        _statusLine.method.c_str(), _statusLine.uri.c_str(), _statusLine.version.c_str());
    string::size_type p = _statusLine.uri.find('?');
    if (p != string::npos) {
        string params = StringUtils::urlDecode(_statusLine.uri.substr(p + 1));
        StringUtils::split(params, '&', '=', _params);
        _statusLine.uri = _statusLine.uri.substr(0, p);
    }
}

bool Request::connectionClose() const {
    string field = Header::field(Header::Connection);
    ConstIt it = _headrs.find(field);
    if (it != _headrs.end() && !strncasecmp(it->second.c_str(), "close", 5)) {
        return true;
    }

    return false;
}

END_NS
