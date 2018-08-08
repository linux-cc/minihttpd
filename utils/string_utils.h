#ifndef __UTILS_STRING_UTILS_H__
#define __UTILS_STRING_UTILS_H__

#include "config.h"
#include <string>

BEGIN_NS(utils)

using std::string;

class StringUtils {
public:
    static string &trim(string &str);
    static string trim(char *str);
    static void split(const string &str, char split, string *vals[], int nval);
};

END_NS
#endif /* ifndef __UTILS_STRING_UTILS_H__ */
