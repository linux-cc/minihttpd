#ifndef __MEMORY_SLAB_MALLOC_H__
#define __MEMORY_SLAB_MALLOC_H__

namespace memory {

class BuddyMalloc;
class SlabMalloc {
public:
    SlabMalloc(BuddyMalloc &buddy);
    void *alloc(int size);
    void free(const void *addr);
    char *dump();

private:
    struct SlabInfo;
    void initPage(int idx, void *page);
    void linkSlab(int idx, SlabInfo *slab);

    enum {
        MAX_FREE_NUM = 32
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
    BuddyMalloc &_buddy;
};

} /* namespace memory */
#endif /* ifndef __MEMORY_SLAB_MALLOC_H__ */
