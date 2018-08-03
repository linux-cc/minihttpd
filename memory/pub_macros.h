#ifndef MEMORY_PUB_MACROS_H
#define MEMORY_PUB_MACROS_H

#include "config.h"

#define SIZE_BITS           32U
#define MAX_FREE_LIST       16
#define PAGE_SIZE           4096 
#define PAGE_MASK           (PAGE_SIZE - 1)
#define ALIGN_PAGE(n)       (((n) + PAGE_MASK) & ~PAGE_MASK)
#define ALIGN8_SIZE         8
#define ALIGN8_MASK         7
#define ALIGN8_SHIFT        3
#define ALIGN8(n)           (((n) + ALIGN8_MASK) & ~ALIGN8_MASK)
#define IS_POW2(n)          (!((n) & ((n) - 1)))
#define LOG2(n)             (SIZE_BITS - __buildin_clz(n) - (IS_POW2(n) ? 1 : 0))
#define LOG2PLUS1(n)        (LOG2(n) + 1)

#define MAX(a, b)           (a) < (b) ? (b) : (a)

inline uint32_t __buildin_clz(uint32_t x) {
    uint32_t n = SIZE_BITS, s = n >> 1, y;
    while (s) {
        y = x >> s;
        if (y) {
            n -= s;
            x = y;
        }
        s >>= 1;
    }
    return n - 1;
}

#endif

