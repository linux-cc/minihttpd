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

    Header() {
        init();
    }
    Header(const string &line);
    Header(const string &field, const string &value);
    Header(Field field, const string &value);
    virtual ~Header() {}
    void set(const string &field, const string &value);
    void set(Field field, const string &value);
    const string &field() const {
        return _field;
    }
    const string &value() const {
        return _value;
    }
    bool empty() const {
        return _field.empty() && _value.empty();
    }
    static void init();
protected:
    string _field;
    string _value;
    static map<int, string> _fields;
};

END_NS
#endif /* ifndef __HTTPD_HEADER_H__ */
