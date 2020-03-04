#ifndef __UTIL_UTIL_H__
#define __UTIL_UTIL_H__

#include <stddef.h>

namespace util {

class String;

size_t kmpSearch(const char *text, const char *pattern);
size_t kmpSearch(const char *text, const char *pattern, size_t plen);
size_t sundaySearch(const char *text, const char *pattern);
size_t sundaySearch(const char *text, const char *pattern, size_t plen);
String urlDecode(const String &str);
void writeLog(const char *func, int line, const char *fmt, ...);

template<typename T>
inline T atomicAddAndFetch(T* ptr, T value) { return __sync_add_and_fetch(ptr, value); }

template<typename T>
inline T atomicFetchAndAdd(T* ptr, T value) { return __sync_fetch_and_add(ptr, value); }

template<typename T>
inline T atomicSubAndFetch(T* ptr, T value) { return __sync_sub_and_fetch(ptr, value); }

template<typename T>
inline T atomicFetchAndSub(T* ptr, T value) { return __sync_fetch_and_sub(ptr, value); }

template<typename T>
inline bool atomicBoolCas(T* ptr, T oldval, T newval) { return __sync_bool_compare_and_swap(ptr, oldval, newval); }

template<typename T>
inline T atomicValCas(T* ptr, T oldval, T newval) { return __sync_val_compare_and_swap(ptr, oldval, newval); }

template<typename T>
inline void atomicInc(T* ptr) { atomicAddAndFetch(ptr, 1); }

template<typename T>
inline T atomicDec(T* ptr) { return atomicSubAndFetch(ptr, 1); }

}

#endif /* __UTIL_UTIL_H__ */
