#ifndef __HTTPD_CONSTANTS_H__
#define __HTTPD_CONSTANTS_H__

#include <string>

namespace httpd {

using std::string;

struct Header {
    enum {
        // general headers
        Cache_Control = 0,
        Connection,
        Date,
        Pragma,
        Trailer,
        Transfer_Encoding,
        Upgrade,
        Via,
        Warning,
        // request headers
        Accept,
        Accept_Charset,
        Accept_Encoding,
        Accept_Language,
        Authorization,
        Expect,
        From,
        Host,
        If_Match,
        If_Modified_Since,
        If_None_Match,
        If_Range,
        If_Unmodified_Since,
        Max_Forwards,
        Proxy_Authorization,
        Range,
        Referer,
        TE,
        User_Agent,
        // response headers
        Accept_Ranges,
        Age,
        ETag,
        Location,
        Proxy_Authenticate,
        Retry_After,
        Server,
        Vary,
        WWW_Authenticate,
        // entity headers
        Allow,
        Content_Disposition,
        Content_Encoding,
        Content_Language,
        Content_Length,
        Content_Location,
        Content_MD5,
        Content_Range,
        Content_Type,
        Expires,
        Last_Modified, 
    };
    static void init();
    static const string &getName(int header);
};

struct ResponseStatus {
    enum {
        // Informational 1xx
        Continue = 100,
        Switching_Protocols, 
        // Successful 2xx
        OK = 200,
        Created,
        Accepted,
        Non_Authoritative_Information,
        No_Content,
        Reset_Content,
        Partial_Content,
        // Redirection 3xx
        Multiple_Choices = 300,
        Moved_Permanently,
        Found,
        See_Other,
        Not_Modified,
        Use_Proxy,
        Unused,
        Temporary_Redirect,
        // Client Error 4xx
        Bad_Request = 400,
        Unauthorized,
        Payment_Required,
        Forbidden,
        Not_Found,
        Method_Not_Allowed,
        Not_Acceptable,
        Proxy_Authentication_Required,
        Request_Timeout,
        Conflict,
        Gone,
        Length_Required,
        Precondition_Failed,
        Request_Entity_Too_Large,
        Request_URI_Too_Long,
        Unsupported_Media_Type,
        Requested_Range_Not_Satisfiable,
        Expectation_Failed,
        // Server Error 5xx
        Internal_Server_Error = 500,
        Not_Implemented,
        Bad_Gateway,
        Service_Unavailable,
        Gateway_Timeout,
        HTTP_Version_Not_Supported, 
    };
    static void init();
    static const string &getReason(int status);
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_CONSTANTS_H__ */
