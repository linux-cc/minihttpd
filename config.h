#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>

#ifdef _DEBUG_
#include <stdio.h>
#ifdef __linux__
#include <sys/sendfile.h>
#define _LOG_(fmt, ...)   printf("[%lu]"fmt, pthread_self(), ##__VA_ARGS__)
#else
#define _LOG_(fmt, ...)   printf("[%lu]"fmt, (intptr_t)pthread_self(), ##__VA_ARGS__)
#endif
#else
#define _LOG_(...)    
#endif

#define CR                  '\r'
#define LF                  '\n'
#define CRLF                "\r\n"
#define BEGIN_NS(name)      namespace myframe { namespace name {
#define END_NS              }}
#define USING_NS(name)      using namespace myframe::name
#define USING_CLASS(ns, c)  using myframe::ns::c
#define MAX(a, b)           ((a) > (b) ? (a) : (b))
#define MIN(a, b)           ((a) > (b) ? (b) : (a))

#ifdef __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4
#define atomic_add_and_fetch(atomic, value)     __sync_add_and_fetch((atomic), (value))
#define atomic_sub_and_fetch(atomic, value)     __sync_sub_and_fetch((atomic), (value))
#define atomic_fetch_and_add(atomic, value)     __sync_fetch_and_add((atomic), (value))
#define atomic_fetch_and_sub(atomic, value)     __sync_fetch_and_sub((atomic), (value))
#define atomic_bool_cas(atomic, oldval, newval) __sync_bool_compare_and_swap((atomic), (oldval), (newval))
#else
static pthread_mutex_t __lock = PTHREAD_MUTEX_INITIALIZER;
template<typename T>
inline T atomic_add_and_fetch(volatile T *atomic, T value) {
	pthread_mutex_lock(&__lock);
	*atomic += value;
	pthread_mutex_unlock(&__lock);
    return *atomic;
}

template<typename T>
inline T atomic_fetch_and_add(volatile T *atomic, T value) {
    T old = *atomic;
	pthread_mutex_lock(&__lock);
	*atomic += value;
	pthread_mutex_unlock(&__lock);
    return old;
}

template<typename T>
inline T atomic_sub_and_fetch(volatile T *atomic, T value) {
	pthread_mutex_lock(&__lock);
	*atomic -= value;
	pthread_mutex_unlock(&__lock);
    return *atomic;
}

template<typename T>
inline T atomic_fetch_and_sub(volatile T *atomic, T value) {
    T old = *atomic;
	pthread_mutex_lock(&__lock);
	*atomic -= value;
	pthread_mutex_unlock(&__lock);
    return old;
}

template<typename T>
inline bool atomic_bool_cas(volatile T *atomic, T oldval, T newval) {
    bool success;
    pthread_mutex_lock(&__lock);
    if((success = (*atomic == oldval)))
        *atomic = newval;
    pthread_mutex_unlock(&__lock);

    return success;
}
#endif /* __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 */
#endif /* ifndef __CONFIG_H__ */
