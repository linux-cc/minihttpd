#include "util/algorithm.h"
#include "util/string.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

namespace util {

static void getNext(uint16_t *next, const char *pattern, size_t plen) {
    next[0] = 0;
    for (int i = 1, k = 0; i < plen; ++i) {
        while (k && pattern[k] != pattern[i]) {
            k = next[k - 1];
        }
        if (pattern[k] == pattern[i]) {
            ++k;
        }
        next[i] = k;
    }
}

size_t kmpSearch(const char *text, const char *pattern) {
    size_t plen = strlen(pattern);
    return kmpSearch(text, pattern, plen);
}

size_t kmpSearch(const char *text, const char *pattern, size_t plen) {
    static uint16_t _next[256];
    size_t tlen = strlen(text);
    getNext(_next, pattern, plen);
    size_t i = 0, k = 0;
    for (; i < tlen; ++i) {
        while (k && pattern[k] != text[i]) {
            k = _next[k - 1];
        }
        if (pattern[k] == text[i]) {
            k++;
        }
        if (k == plen) {
            break;
        }
    }

    return k == plen ? i - k + 1: -1;
}

size_t sundaySearch(const char *text, const char *pattern) {
    size_t plen = strlen(pattern);
    return sundaySearch(text, pattern, plen);
}

size_t sundaySearch(const char *text, const char *pattern, size_t plen) {
    static uint16_t _move[128];
    size_t tlen = strlen(text);
    for (int i = 0; i < 128; ++i) {
        _move[i] = plen + 1;
    }
    for (int i = 0; i < plen; ++i) {
        _move[int(pattern[i])] = plen - i;
    }
    
    int s = 0, j;
    while (s <= tlen - plen) {
        j = 0;
        while (text[s+j] == pattern[j]) {
            ++j;
            if (j >= plen) {
                return s;
            }
        }
        s += _move[int(text[s + plen])];
    }
    
    return -1;
}

static char hexToChar(char hex) {
    if (hex >= 'A' && hex <= 'F') {
        return hex - 'A';
    }
    if (hex >= 'a' && hex <= 'f') {
        return hex - 'a';
    }

    return hex - '0';
}

String urlDecode(const String &str) {
    String decode("");
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

void writeLog(const char *func, int line, const char *fmt, ...) {
    char buffer[128];
    va_list vp;
    va_start(vp, fmt);
    size_t p1 = sundaySearch(func, "::");
    size_t p2 = sundaySearch(func + p1, "(");
    memcpy(buffer, func + p1 + 2, p2 - 2);
    buffer[p2 - 2] = 0;
        printf("[%s:%d]", buffer, line);
    vprintf(fmt, vp);
    printf("\n");
    va_end(vp);
}

}
