#ifndef __MEMORY_BUDDY_H__
#define __MEMORY_BUDDY_H__

#include "config.h"

#define SIZE_BITS           32U
#define MAX_FREE_NUM        16
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

BEGIN_NS(memory)

class Buddy {
public:
    Buddy(): _buffer(NULL), _tree(NULL), _depth(0) {}
    explicit Buddy(size_t pages): _buffer(NULL), _tree(NULL), _depth(0) {
        init(pages);
    }
    ~Buddy();
    void init(size_t pages);
    void *alloc(size_t pages);
    void free(void *addr);
    char *dump();
    char *buffer() { return _buffer; }

private:
    char *_buffer;
    char *_pages;
    uint8_t *_tree;
    uint8_t _depth;
};

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

END_NS
#endif /* ifndef __MEMORY_BUDDY_H__ */
