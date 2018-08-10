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
        // general headers
        Cache_Control, Connection, Date, Pragma, Trailer, Transfer_Encoding, Upgrade, Via, Warning,
        // request headers
        Accept, Accept_Charset, Accept_Encoding, Accept_Language, Authorization, Expect, From, Host,
        If_Match, If_Modified_Since, If_None_Match, If_Range, If_Unmodified_Since, Max_Forwards,
        Proxy_Authorization, Range, Referer, TE, User_Agent,
        // response headers
        Accept_Ranges, Age, ETag, Location, Proxy_Authenticate, Retry_After, Server, Vary, WWW_Authenticate,
        // entity headers
        Allow, Content_Encoding, Content_Language, Content_Length, Content_Location, Content_MD5, Content_Range,
        Content_Type, Expires, Last_Modified, 
    };

    Header() {}
    Header(const string &line);
    Header(const string &field, const string &value);
    Header(Field field, const string &value);

    void set(const string &field, const string &value);
    void set(Field field, const string &value);

    const string &field() const {
        return _field;
    }
    const string &value() const {
        return _value;
    }
    string toString() const {
        return _field + " : " + _value;
    }
    bool empty() const {
        return _field.empty() && _value.empty();
    }
    static string field(int field);
protected:
    string _field;
    string _value;
};

END_NS
#endif /* ifndef __HTTPD_HEADER_H__ */
