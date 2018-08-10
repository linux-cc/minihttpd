#ifndef __UTILS_STRING_UTILS_H__
#define __UTILS_STRING_UTILS_H__

#include "config.h"
#include <string>
#include <map>

BEGIN_NS(utils)

using std::string;
using std::map;

class StringUtils {
public:
    static string &trim(string &str);
    static string trim(char *str);
    static void split(const string &str, char delim, string *vals[], int nval);
    static void split(const string &str, char delim1, char delim2, map<string, string> &val);
    static string urlDecode(const string &str);
    static char hexToChar(char hex);
};

END_NS
#endif /* ifndef __UTILS_STRING_UTILS_H__ */
