#ifndef __MEMORY_SLAB_MALLOC_H__
#define __MEMORY_SLAB_MALLOC_H__

namespace memory {

class Buddy;
class SlabMalloc {
public:
    SlabMalloc(Buddy &buddy);
    void *alloc(int size);
    void free(void *addr);
    char *dump();

private:
    struct SlabInfo;
    void initPage(int idx, void *page);
    void linkSlab(int idx, SlabInfo *slab);

    enum {
        MAX_FREE_NUM = 16
    };
    struct Chunk {
        Chunk *next;
    };
    struct SlabInfo {
        short chunkSize;
        short allocated;
        Chunk *free;
        SlabInfo *prev;
        SlabInfo *next;
    };
    SlabInfo *_free[MAX_FREE_NUM];
    Buddy &_buddy;
};

} /* namespace memory */
#endif /* ifndef __MEMORY_SLAB_MALLOC_H__ */
