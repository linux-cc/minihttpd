#ifndef __MEMORY_FIXED_MALLOC_H__
#define __MEMORY_FIXED_MALLOC_H__

#include "config.h"

BEGIN_NS(memory)

class Buddy;

class FixedMalloc {
public:
    FixedMalloc(Buddy &buddy);

    void *alloc(size_t size);
    void *alloc() { return alloc(_elem); }
    void free(void *addr);
    bool init(size_t size, size_t elem);
    void destroy();

private:
    Buddy &_buddy;
    size_t _elem;
    size_t _size;
    size_t _free;
    size_t _used;
    char *_buffer;
};

END_NS
#endif /* ifndef __MEMORY_FIXED_MALLOC_H__ */
