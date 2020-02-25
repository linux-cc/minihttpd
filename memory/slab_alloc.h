#ifndef __MEMORY_SLAB_ALLOC_H__
#define __MEMORY_SLAB_ALLOC_H__

#include <stddef.h>

namespace memory {

class BuddyAlloc;
class SlabAlloc {
public:
    SlabAlloc(BuddyAlloc &buddy);
    void *alloc(size_t size);
    void free(const void *addr, size_t size);
    static int magic() { return SLAB_MAGIC; }
    static int maxSlabSize() { return SLAB_MAX_SIZE; }
    char *dump();

private:
    struct SlabInfo;
    void initPage(int idx, void *page);
    void linkSlab(int idx, SlabInfo *slab);

    enum {
        ALIGN8_SHIFT = 3,
        ALIGN8_MASK = 7,
        ALIGN8_SIZE = 8,
        SLAB_FREE_NUM = 32,
        SLAB_MAX_SIZE = SLAB_FREE_NUM << ALIGN8_SHIFT,
        SLAB_MAGIC = 0xccddeeff,
    };
    struct Chunk {
        Chunk *next;
    };
    struct SlabInfo {
        Chunk *free;
        SlabInfo *prev;
        SlabInfo *next;
        short allocated;
    };
    SlabInfo *_free[SLAB_FREE_NUM];
    BuddyAlloc &_buddy;
    int _pageSize;
};

} /* namespace memory */
#endif /* ifndef __MEMORY_SLAB_MALLOC_H__ */
