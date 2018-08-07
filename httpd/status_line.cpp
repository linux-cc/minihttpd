#include "httpd/status_line.h"

BEGIN_NS(httpd)

static void __parse(const string &line, string **strs) {
    string::size_type p1 = 0, p2;
    int i = 0;
    while ((p2 = line.find(' ', p1)) != string::npos) {
        *strs[i++] = line.substr(p1, p2);
        p1 = p2 + 1;
    }
    if (p1 < line.length()) {
        *strs[i] = line.substr(p1);
    }
}

void RequestStatusLine::parse(const string &line) {
    string *strs[] = { &_method, &_uri, &_version };
    __parse(line, strs);
}

void ResponseStatusLine::parse(const string &line) {
    string *strs[] = { &_version, &_status, &_cause };
    __parse(line, strs);
}

END_NS
