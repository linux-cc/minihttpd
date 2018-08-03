#ifndef __MEMORY_SLAB_MALLOC_H__
#define __MEMORY_SLAB_MALLOC_H__

#include "buddy.h"

BEGIN_NS(memory)

class SlabMalloc {
public:
    SlabMalloc(Buddy *_buddy);
    void *alloc(size_t size);
    void free(void *addr);
    char *dump();

private:
    struct Info;
    void initPage(size_t idx, void *page);
    void linkSlab(size_t idx, Info *slab);

private:
    struct Chunk {
        Chunk *next;
    };
    struct Info {
        uint16_t chunkSize;
        uint16_t allocated;
        Chunk *free;
        Info *prev;
        Info *next;
    };
    Buddy *buddy;
    Info *freeList[MAX_FREE_LIST];
};

END_NS

#endif
