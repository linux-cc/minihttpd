#ifndef __MEMORY_DL_MALLOC_H__
#define __MEMORY_DL_MALLOC_H__

#include <stdint.h>
#include <stddef.h>

namespace memory {

class Buddy;

class DlMalloc {
public:
    DlMalloc(Buddy &buddy);
    void* alloc(size_t size);
    void free(void* addr);
    char* dump();

private:
    struct Chunk;
    struct Segment;
    uint32_t computeTreeIndex(size_t nb);
    Chunk* getFitChunk(size_t nb, size_t idx, size_t &rsize);
    void unlinkChunk(Chunk* chunk);
    void insertChunk(Chunk* chunk, size_t nb);
    void* sysAlloc(size_t nb);
    void mergeSeg(void* base, size_t size);
    void appendSeg(Segment* near1, Segment* near2, void* base, size_t size);
    void prependSeg(Segment* near, void* base, size_t size);
    void addSegment(void* base, size_t size);
    void initTop(void* base, size_t size);
    void dumpChunk(char* buf);
    int dumpChunk(char* buf, Chunk* chunk);
    Segment* getNear(void* base, size_t size, bool append);

    size_t padRequest(size_t size) {
        return (size + sizeof(size_t) + ALIGN_MASK) & ~ALIGN_MASK;
    }
    size_t minChunkSize() {
        return padRequest(1 << TREE_SHIFT);
    }
    size_t lshForTreeIdx(uint32_t i) {
        return SIZE_T_BITSIZE - ((i >> 1) + TREE_SHIFT - 2);
    }
    size_t chunkSize(Chunk* chunk) {
        return chunk->head & ~BIT_RCP;
    }
    Chunk* prevChunk(Chunk* chunk) {
        return (Chunk*)((char*)chunk - chunk->prevSize);
    }
    Chunk* nextChunk(Chunk* chunk) {
        return (Chunk*)((char*)chunk + chunkSize(chunk));
    }
    Chunk* nextChunk(void* chunk, size_t size) {
        return (Chunk*)((char*)chunk + size);
    }
    int getLeastBit(int i) {
        return i & -i;
    }
    int getLeftBits(int i) {
        return (((1 << i) << 1) | -((1 << i) << 1)) & _treeMap;
    }
    Chunk* leftMostChild(Chunk* chunk) {
        return chunk->child[0] ? chunk->child[0] : chunk->child[1];
    }
    Chunk*& rightMostChild(Chunk* chunk) {
        return chunk->child[1] ? chunk->child[1] : chunk->child[0];
    }
    void markTreeMap(int i) {
        _treeMap |= (1 << i);
    }
    void clearTreeMap(int i) {
        _treeMap &= ~(1 << i);
    }
    bool treeMapIsMark(int i) {
        return _treeMap & (1 << i);
    }
    void setChunkUsed(Chunk* chunk, size_t size) {
        chunk->head = size | BIT_CP;
    }
    void setChunkUsedWithP(Chunk* chunk, size_t size) {
        chunk->head = size | BIT_CP, nextChunk(chunk, size)->head |= BIT_P;
    }
    void setChunkFree(Chunk* chunk, size_t size, Chunk *next) {
        next->head &= ~BIT_P, setChunkFree(chunk, size);
    }
    void setChunkFree(Chunk* chunk, size_t size) {
        chunk->head = size | BIT_P, nextChunk(chunk, size)->prevSize = size;
    }
    void* chunk2mem(Chunk* chunk) {
        return (char*)chunk + (sizeof(size_t) << 1);
    }
    Chunk* mem2chunk(void* mem) {
        return (Chunk*)((char*)mem - (sizeof(size_t) << 1));
    }
    bool holdsTop(Segment* seg) {
        return (char*)_top >= seg->base && (char*)_top < seg->base + seg->size;
    }
    bool bitCSet(Chunk* chunk) {
        return chunk->head & BIT_C;
    }
    bool bitPSet(Chunk* chunk) {
        return chunk->head & BIT_P;
    }
    size_t topSize() {
        return _top ? chunkSize(_top) : 0;
    }
    void setTop(Chunk* chunk, size_t size) {
        _top = chunk, _top->head = size | BIT_P, nextChunk(_top, size)->prevSize = size;
    }
    size_t footSize() {
        return padRequest(sizeof(Segment));
    }
    
private:
    enum {
        BIT_P = 1,
        BIT_C = 2,
        BIT_R = 4,
        BIT_CP = 3,
        BIT_RCP = 7,
    };
    enum {
        TREE_SHIFT = 7,
        TREE_NUM = 16,
        ALIGN_MASK = (sizeof(void*) << 1) - 1,
        SIZE_T_BITSIZE = sizeof(size_t) * __CHAR_BIT__ - 1,
    };

    struct Chunk {
        size_t prevSize;
        size_t head;
        Chunk* prev;
        Chunk* next;
        Chunk* child[2];
        Chunk* parent;
        uint32_t index;
    };
    struct Segment {
        char* base;
        size_t size;
        Segment* prev;
        Segment* next;
    };
    Buddy& _buddy;
    Chunk* _top;
    Chunk* _tree[TREE_NUM];
    Segment *_head;
    uint32_t _treeMap;
};

} /* namespace memory */
#endif /* ifndef __MEMORY_DL_MALLOC_H__ */
