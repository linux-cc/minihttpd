#include "utils/string_utils.h"

BEGIN_NS(utils)

string &StringUtils::trim(string &str) {
    const char *base = str.c_str();
    const char *p1 = base;
    const char *p2 = base + str.length() - 1;
    while(isspace(*p1)) p1++;
    while(isspace(*p2)) p2--;
    str.erase(0, p1 - base);
    str.erase(p2 - base + 1);

    return str;
}

string StringUtils::trim(char *str) {
    char *base = str;
    char *p1 = base;
    char *p2 = base + strlen(str) - 1;
    while(isspace(*p1)) p1++;
    while(isspace(*p2)) p2--;
    *++p2 = '\0';
    memmove(str, p1, p2 - p1 + 1);

    return str;
}

void StringUtils::split(const string &str, char split, string *vals[], int nval) {
    string::size_type p1 = 0, p2;
    int i = 0;
    while ((p2 = str.find(split, p1)) != string::npos && nval--) {
        *vals[i++] = str.substr(p1, p2);
        p1 = p2 + 1;
    }
    if (nval > 0 && p1 < str.length()) {
        *vals[i] = str.substr(p1);
    }
}

END_NS
