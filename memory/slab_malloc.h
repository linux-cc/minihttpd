#ifndef __MEMORY_SLAB_MALLOC_H__
#define __MEMORY_SLAB_MALLOC_H__

#include <stdint.h>
#include <stddef.h>

namespace memory {

class Buddy;

class SlabMalloc {
public:
    SlabMalloc(Buddy &buddy);
    void* alloc(size_t size);
    void free(void *addr);
    char* dump();

private:
    struct Info;
    void initPage(size_t idx, void* page);
    void linkSlab(size_t idx, Info* slab);

private:
    enum {
        MAX_FREE_NUM = 16
    };
    struct Chunk {
        Chunk* next;
    };
    struct Info {
        uint16_t chunkSize;
        uint16_t allocated;
        Chunk* free;
        Info* prev;
        Info* next;
    };
    Buddy& _buddy;
    Info* _free[MAX_FREE_NUM];
};

} /* namespace memory */
#endif /* ifndef __MEMORY_SLAB_MALLOC_H__ */
