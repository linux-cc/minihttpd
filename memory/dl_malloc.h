#ifndef __MEMORY_DL_MALLOC_H__
#define __MEMORY_DL_MALLOC_H__

#include "memory/buddy.h"

BEGIN_NS(memory)

class DlMalloc {
public:
    DlMalloc(Buddy &buddy);
    void *alloc(size_t size);
    void free(void *addr);
    char *dump();

private:
    struct Chunk;
    struct Segment;
    size_t computeFreeIndex(size_t nb);
    Chunk *getFitChunk(size_t nb, size_t idx, size_t &rsize);
    void unlinkChunk(Chunk *chunk);
    void insertChunk(Chunk *chunk, size_t nb);
    void *sysAlloc(size_t nb);
    Segment *getTopSeg();
    void *mergeSeg(void *base, size_t size, size_t nb);
    void *prependSeg(void *newBase, void *oldBase, size_t nb);
    void addSegment(void *base, size_t size);
    void dumpChunk(char *buf);
    int dumpChunk(char *buf, Chunk *chunk);

private:
    struct Chunk {
        size_t prev_foot;
        size_t head;
        Chunk *prev;
        Chunk *next;
        Chunk *child[2];
        Chunk *parent;
        uint32_t index;
    };
    struct Segment {
        char *base;
        size_t size;
        Segment *next;
    };
    Buddy &_buddy;
    uint32_t _freeMap;
    Chunk *_top;
    uint32_t _topSize;
    Chunk *_free[MAX_FREE_NUM];
    Segment _seg;
};

END_NS
#endif /* ifndef __MEMORY_DL_MALLOC_H__ */
