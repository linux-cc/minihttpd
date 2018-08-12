#include "utils/string_utils.h"

BEGIN_NS(utils)

string &StringUtils::trim(string &str) {
    const char *base = str.c_str();
    const char *p1 = base;
    const char *p2 = base + str.length() - 1;
    while(isspace(*p1)) p1++;
    while(isspace(*p2)) p2--;
    str.erase(p2 - base + 1);
    str.erase(0, p1 - base);

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

void StringUtils::split(const string &str, char delim, string *vals[], int nval) {
    string::size_type p1 = 0, p2;
    int i = 0;
    while (nval--) {
        p2 = str.find(delim, p1);
        *vals[i++] = p2 == string::npos ? str.substr(p1) : str.substr(p1, p2 - p1);
        if (p2 == string::npos) {
            break;
        }
        p1 = p2 + 1;
        if (p1 >= str.length()) {
            break;
        }
    }
}

void StringUtils::split(const string &str, char delim1, char delim2, map<string, string> &val) {
    string::size_type p1 = 0, p2;
    while (true) {
        p2 = str.find(delim1, p1);
        string kv = p2 == string::npos ? str.substr(p1) : str.substr(p1, p2 - p1);
        string::size_type p3 = kv.find(delim2);
        string k = p3 != string::npos ? kv.substr(0, p3) : kv;
        string v = p3 != string::npos ? kv.substr(p3 + 1) : "";
        val[k] = v;
        p1 = p2 + 1;
        if (p2 == string::npos || p1 >= str.length()) {
            break;
        }
    }
}

string StringUtils::urlDecode(const string &str) {
    string decode("");
    size_t length = str.length();
    for (size_t i = 0; i < length; ++i) {
        if (str[i] == '+') {
            decode += ' ';
        } else if (str[i] == '%')
        {
            uint8_t high = hexToChar(str[++i]);
            uint8_t low = hexToChar(str[++i]);
            decode += (high << 4) | low;
        } else {
            decode += str[i];
        }
    }

    return decode;
 
}

char StringUtils::hexToChar(char hex) {
    if (hex >= 'A' && hex <= 'F') {
        return hex - 'A';
    }
    if (hex >= 'a' && hex <= 'f') {
        return hex - 'a';
    }

    return hex - '0';
}

string StringUtils::itoa(int i) {
    char temp[16] = { 0 };
    sprintf(temp, "%d", i);
    return temp;
}
END_NS
