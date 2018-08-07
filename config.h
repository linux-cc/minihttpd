#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifdef _DEBUG_
#include <stdio.h>
#define __LOG__         printf
#endif

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
#include <pthread.h>
static pthread_mutex_t __lock = PTHREAD_MUTEX_INITIALIZER;

inline int atomic_add_and_fetch(volatile int *atomic, int value) {
	pthread_mutex_lock(&__lock);
	*atomic += value;
	pthread_mutex_unlock(&__lock);
    return *atomic;
}

inline int atomic_fetch_and_add(volatile int *atomic, int value) {
    int old = *atomic;
	pthread_mutex_lock(&__lock);
	*atomic += value;
	pthread_mutex_unlock(&__lock);
    return old;
}

inline int atomic_sub_and_fetch(volatile int *atomic, int value) {
	pthread_mutex_lock(&__lock);
	*atomic -= value;
	pthread_mutex_unlock(&__lock);
    return *atomic;
}

inline int atomic_fetch_and_sub(volatile int *atomic, int value) {
    int old = *atomic;
	pthread_mutex_lock(&__lock);
	*atomic -= value;
	pthread_mutex_unlock(&__lock);
    return old;
}
inline bool atomic_bool_cas(volatile int *atomic, int oldval, int newval) {
    bool success;
    pthread_mutex_lock(&__lock);
    if((success = (*atomic == oldval)))
        *atomic = newval;
    pthread_mutex_unlock(&__lock);

    return success;
}
#endif /* __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 */
#endif /* ifndef __CONFIG_H__ */
