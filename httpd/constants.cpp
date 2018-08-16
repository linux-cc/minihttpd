#include "httpd/constants.h"
#include <map>

BEGIN_NS(httpd)

using std::map;

#define __INIT__(code, replace, result)\
do {\
    string strcode = #code;\
    string::size_type p;\
    while ((p = strcode.find('_')) != string::npos)\
        strcode[p] = replace;\
    result[code] = strcode;\
}while (0)

static map<int, string> __fields;
static map<int, string> __status;

void Header::initFieldName() {
    if (!__fields.empty()) {
        return;
    }
    // general headers
    __INIT__(Cache_Control, '-', __fields);
    __INIT__(Connection, '-', __fields);
    __INIT__(Date, '-', __fields);
    __INIT__(Pragma, '-', __fields);
    __INIT__(Trailer, '-', __fields);
    __INIT__(Transfer_Encoding, '-', __fields);
    __INIT__(Upgrade, '-', __fields);
    __INIT__(Via, '-', __fields);
    __INIT__(Warning, '-', __fields);
    // request headers
    __INIT__(Accept, '-', __fields);
    __INIT__(Accept_Charset, '-', __fields);
    __INIT__(Accept_Encoding, '-', __fields);
    __INIT__(Accept_Language, '-', __fields);
    __INIT__(Authorization, '-', __fields);
    __INIT__(Expect, '-', __fields);
    __INIT__(From, '-', __fields);
    __INIT__(Host, '-', __fields);
    __INIT__(If_Match, '-', __fields);
    __INIT__(If_Modified_Since, '-', __fields);
    __INIT__(If_None_Match, '-', __fields);
    __INIT__(If_Range, '-', __fields);
    __INIT__(If_Unmodified_Since, '-', __fields);
    __INIT__(Max_Forwards, '-', __fields);
    __INIT__(Proxy_Authorization, '-', __fields);
    __INIT__(Range, '-', __fields);
    __INIT__(Referer, '-', __fields);
    __INIT__(TE, '-', __fields);
    __INIT__(User_Agent, '-', __fields);
    // response headers
    __INIT__(Accept_Ranges, '-', __fields);
    __INIT__(Age, '-', __fields);
    __INIT__(ETag, '-', __fields);
    __INIT__(Location, '-', __fields);
    __INIT__(Proxy_Authenticate, '-', __fields);
    __INIT__(Retry_After, '-', __fields);
    __INIT__(Server, '-', __fields);
    __INIT__(Vary, '-', __fields);
    __INIT__(WWW_Authenticate, '-', __fields);
    // entity headers
    __INIT__(Allow, '-', __fields);
    __INIT__(Content_Encoding, '-', __fields);
    __INIT__(Content_Language, '-', __fields);
    __INIT__(Content_Length, '-', __fields);
    __INIT__(Content_Location, '-', __fields);
    __INIT__(Content_MD5, '-', __fields);
    __INIT__(Content_Range, '-', __fields);
    __INIT__(Content_Type, '-', __fields);
    __INIT__(Expires, '-', __fields);
    __INIT__(Last_Modified, '-', __fields);
}

void ResponseStatus::initStatusReason() {
    if (!__status.empty()) {
        return;
    }
    // Informational 1xx
    __INIT__(Continue, ' ', __status);
    __INIT__(Switching_Protocols, ' ', __status);
    // Successful 2xx
    __INIT__(OK, ' ', __status);
    __INIT__(Created, ' ', __status);
    __INIT__(Accepted, ' ', __status);
    __INIT__(Non_Authoritative_Information, ' ', __status);
    __INIT__(No_Content, ' ', __status);
    __INIT__(Reset_Content, ' ', __status);
    __INIT__(Partial_Content, ' ', __status);
    // Redirection 3xx
    __INIT__(Multiple_Choices, ' ', __status);
    __INIT__(Moved_Permanently, ' ', __status);
    __INIT__(Found, ' ', __status);
    __INIT__(See_Other, ' ', __status);
    __INIT__(Not_Modified, ' ', __status);
    __INIT__(Use_Proxy, ' ', __status);
    __INIT__(Unused, ' ', __status);
    __INIT__(Temporary_Redirect, ' ', __status);
    // Client Error 4xx
    __INIT__(Bad_Request, ' ', __status);
    __INIT__(Unauthorized, ' ', __status);
    __INIT__(Payment_Required, ' ', __status);
    __INIT__(Forbidden, ' ', __status);
    __INIT__(Not_Found, ' ', __status);
    __INIT__(Method_Not_Allowed, ' ', __status);
    __INIT__(Not_Acceptable, ' ', __status);
    __INIT__(Proxy_Authentication_Required, ' ', __status);
    __INIT__(Request_Timeout, ' ', __status);
    __INIT__(Conflict, ' ', __status);
    __INIT__(Gone, ' ', __status);
    __INIT__(Length_Required, ' ', __status);
    __INIT__(Precondition_Failed, ' ', __status);
    __INIT__(Request_Entity_Too_Large, ' ', __status);
    __INIT__(Request_URI_Too_Long, ' ', __status);
    __INIT__(Unsupported_Media_Type, ' ', __status);
    __INIT__(Requested_Range_Not_Satisfiable, ' ', __status);
    __INIT__(Expectation_Failed, ' ', __status);
    // Server Error 5xx
    __INIT__(Internal_Server_Error, ' ', __status);
    __INIT__(Not_Implemented, ' ', __status);
    __INIT__(Bad_Gateway, ' ', __status);
    __INIT__(Service_Unavailable, ' ', __status);
    __INIT__(Gateway_Timeout, ' ', __status);
    __INIT__(HTTP_Version_Not_Supported, ' ', __status);
}

const string &getFieldName(int header) {
    return __fields[header];
}

const string &getStatusReason(int status) {
    return __status[status];
}

END_NS
