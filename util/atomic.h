#ifndef __UTIL_ATOMIC_H__
#define __UTIL_ATOMIC_H__

namespace util {

#ifdef __GNUC__
template<typename T>
inline T atomicAddAndFetch(T* ptr, T value) {
    return __sync_add_and_fetch(ptr, value);
}

template<typename T>
inline T atomicFetchAndAdd(T* ptr, T value) {
    return __sync_fetch_and_add(ptr, value);
}

template<typename T>
inline T atomicSubAndFetch(T* ptr, T value) {
    return __sync_sub_and_fetch(ptr, value);
}

template<typename T>
inline T atomicFetchAndSub(T* ptr, T value) {
    return __sync_fetch_and_sub(ptr, value);
}

template<typename T>
inline bool atomicBoolCas(T* ptr, T oldval, T newval) {
    return __sync_bool_compare_and_swap(ptr, oldval, newval);
}

template<typename T>
inline T atomicValCas(T* ptr, T oldval, T newval) {
    return __sync_val_compare_and_swap(ptr, oldval, newval);
}

#else
#include "thread/thread.h"
USING_CLASS(thread, Mutex);
USING_CLASS(thread, AutoMutex);
static Mutex __lock;

template<typename T>
inline T atomicAddAndFetch(T* ptr, T value) {
    AutoMutex m(__lock);
	return (*ptr += value);
}

template<typename T>
inline T atomicFetchAndAdd(T* ptr, T value) {
    T old = *ptr;
    AutoMutex m(__lock);
	*ptr += value;

    return old;
}

template<typename T>
inline T atomicSubAndFetch(T* ptr, T value) {
    AutoMutex m(__lock);

    return (*ptr -= value);
}

template<typename T>
inline T atomicFetchAndSub(T* ptr, T value) {
    T old = *ptr;
    AutoMutex m(__lock);
	*ptr -= value;

    return old;
}

template<typename T>
inline bool atomicBoolCas(T* ptr, T oldval, T newval) {
    AutoMutex m(__lock);

    return *ptr == oldval ? (*ptr = newval, true) : false;
}

template<typename T>
inline T atomicValCas(T* ptr, T oldval, T newval) {
    AutoMutex m(__lock);

    return *ptr == oldval ? (*ptr = newval, oldval) : oldval;
}
#endif /* __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 */

template<typename T>
inline void atomicInc(T* ptr) {
    atomicAddAndFetch(ptr, 1);
}

template<typename T>
inline T atomicDec(T* ptr) {
    return atomicSubAndFetch(ptr, 1);
}

} /* namespace util */
#endif /* __UTIL_ATOMIC_H__ */

