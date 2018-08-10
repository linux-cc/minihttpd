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

void Request::parseUriParams() {
    string uri = StringUtils::urlDecode(_statusLine.uri());
    StringUtils::split(uri, '&', '=', _params);
}

END_NS
