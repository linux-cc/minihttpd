#ifndef __UTIL_ALGORITHM_H__
#define __UTIL_ALGORITHM_H__

#include <stddef.h>

namespace util {

size_t kmpSearch(const char *text, const char *pattern);
size_t kmpSearch(const char *text, const char *pattern, size_t plen);
size_t sundaySearch(const char *text, const char *pattern);
size_t sundaySearch(const char *text, const char *pattern, size_t plen);

}

#endif /* __UTIL_ALGORITHM_H__ */
