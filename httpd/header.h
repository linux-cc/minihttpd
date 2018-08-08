#ifndef __HTTPD_HEADER_H__
#define __HTTPD_HEADER_H__

#include "config.h"
#include <string>
#include <map>

BEGIN_NS(httpd)

using std::string;
using std::map;

class Header {
public:
    enum Field {
        Cache_Control = 0,
        Connection,
        Date,
        Pragma,
        Trailer,
        Transfer_Encoding,
        Upgrade,
        Via,
        Warning,
    };

    Header() {}
    Header(const string &field, const string &value): _field(field), _value(value) {}
    Header(Field field, const string &value): _value(value) {
        _field = _fields[field];
    }
    virtual ~Header() {}
    void set(const string &field, const string &value) {
        _field = field, _value = value;
    }
    void set(Field field, const string &value) {
        _field = _fields[field], _value = value;
    }
    const string &field() const {
        return _field;
    }
    const string &value() const {
        return _value;
    }
    static void init();
protected:
    string _field;
    string _value;
    static map<int, string> _fields;
};

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

END_NS
#endif /* ifndef __HTTPD_HEADER_H__ */
