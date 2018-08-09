#include "httpd/header.h"
#include "utils/string_utils.h"

BEGIN_NS(httpd)

USING_CLASS(utils, StringUtils);

map<int, string> Header::_fields;

void Header::init() {
#define INIT_FIELD(field)       _fields[field] = #field
    if (!_fields.empty()) {
        return;
    }
    _fields[Cache_Control] = "Cache-Control";
    _fields[Transfer_Encoding] = "Transfer-Encoding",
        INIT_FIELD(Connection);
    INIT_FIELD(Date);
    INIT_FIELD(Pragma);
    INIT_FIELD(Trailer);
    INIT_FIELD(Upgrade);
    INIT_FIELD(Via);
    INIT_FIELD(Warning);
#undef INIT_FIELD
}

Header::Header(const string &line) {
    init();
    string *strs[] = { &_field, &_value };
    StringUtils::split(line, ':', strs, 2);
    StringUtils::trim(_field);
    StringUtils::trim(_value);
}

Header::Header(const string &field, const string &value):
_field(field),
_value(value) {
    init();
    StringUtils::trim(_field);
    StringUtils::trim(_value);
}

Header::Header(Field field, const string &value): _value(value) {
    init();
    _field = _fields[field];
    StringUtils::trim(_value);
}

void Header::set(const string &field, const string &value) {
    StringUtils::trim(_field = field);
    StringUtils::trim(_value = value);
}

void Header::set(Field field, const string &value) {
    _field = _fields[field];
    StringUtils::trim(_value = value);
}

END_NS
