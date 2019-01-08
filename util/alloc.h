#ifndef __UTIL_ALLOC_H__
#define __UTIL_ALLOC_H__

#include <new>
#include "util/util.h"

namespace util {

template <typename T1, typename T2>
inline T2* construct(T1* p, const T2& value) {
    return new (p) T2(value);
}

template <typename T>
inline T* construct(T* p) {
    return new (p) T();
}

template <typename T>
inline void destruct(T* p) {
    __destruct(p, IsTrivial<T>());
}

template <typename T>
inline void __destruct(T* p, TrueType) {
}

template <typename T>
inline void __destruct(T* p, FalseType) {
    p->~T();
}

} /* namespace util */

#endif /* __UTIL_ALLOC_H__ */

