#ifndef __MEMORY_DL_MALLOC_H__
#define __MEMORY_DL_MALLOC_H__

#include <stdlib.h>

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
    uint32_t computeFreeIndex(size_t nb);
    Chunk* getFitChunk(size_t nb, size_t idx, size_t &rsize);
    void unlinkChunk(Chunk* chunk);
    void insertChunk(Chunk* chunk, size_t nb);
    void* sysAlloc(size_t nb);
    Segment* getTopSeg();
    void* mergeSeg(void* base, size_t size, size_t nb);
    void* prependSeg(void* newBase, void* oldBase, size_t nb);
    void addSegment(void* base, size_t size);
    void initTop(void* base, size_t size);
    void dumpChunk(char* buf);
    int dumpChunk(char* buf, Chunk* chunk);

    size_t padRequest(size_t size) {
        return (size + sizeof(size_t) + ALIGN_MASK) & ~ALIGN_MASK;
    }
    size_t minChunkSize() {
        return (sizeof(Chunk) + ALIGN_MASK) & ~ALIGN_MASK;
    }
    size_t lshForTreeIdx(uint32_t i) {
        return SIZE_T_BITSIZE - ((i >> 1) + TREEBIN_SHIFT - 2);
    }
    size_t chunkSize(Chunk* chunk) {
        return chunk->head & ~BIT_RCP;
    }
    Chunk* nextChunk(Chunk* chunk) {
        return (Chunk*)((char*)chunk + chunkSize(chunk));
    }
    Chunk* nextChunk(Chunk* chunk, size_t size) {
        return (Chunk*)((char*)chunk + size);
    }
    int getLeastBit(int i) {
        return i & -i;
    }
    int getLeftBits(int i) {
        return (((1 << i) << 1) | -((1 << i) << 1)) & _freeMap;
    }
    Chunk* leftMostChild(Chunk* chunk) {
        return chunk->child[0] ? chunk->child[0] : chunk->child[1];
    }
    Chunk*& rightMostChild(Chunk* chunk) {
        return chunk->child[1] ? chunk->child[1] : chunk->child[0];
    }
    Chunk* chunkPlusOffset(Chunk* chunk, size_t offset) {
        return (Chunk*)((char*)chunk + offset);
    }
    Chunk* chunkMinusOffset(Chunk* chunk, size_t offset) {
        return (Chunk*)((char*)chunk - offset);
    }
    void clearBitMap(int i) {
        _freeMap &= ~(1 << i);
    }
    void setCPPBits(Chunk* chunk, size_t size) {
        chunk->head = size | BIT_CP, nextChunk(chunk, size)->head |= BIT_P;
    }
    void setHeadPAndFoot(Chunk* chunk, size_t size) {
        chunk->head = size | BIT_P, nextChunk(chunk, size)->prev_foot = size;
    }
    void setHeadCP(Chunk* chunk, size_t size) {
        chunk->head = size | BIT_CP;
    }
    void setFreeWithP(Chunk* chunk, size_t size, Chunk *next) {
        next->head &= ~BIT_P, setHeadPAndFoot(chunk, size);
    }
    void* chunk2mem(Chunk* chunk) {
        return (char*)chunk + (sizeof(size_t) << 1);
    }
    Chunk* mem2chunk(void* mem) {
        return (Chunk*)((char*)mem - (sizeof(size_t) << 1));
    }
    void markBitMap(int i) {
        _freeMap |= (1 << i);
    }
    bool bitMapIsMark(int i) {
        return _freeMap & (1 << i);
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
    size_t getTopSize() {
        return _top ? chunkSize(_top) : 0;
    }
    void setTop(Chunk* chunk, size_t size) {
        _top = chunk, _top->head = size | BIT_P;
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
        TREEBIN_SHIFT = 7,
        MAX_FREE_NUM = 16,
        ALIGN_MASK = (sizeof(void*) << 1) - 1,
        SIZE_T_BITSIZE = sizeof(size_t) * __CHAR_BIT__ - 1,
    };

    struct Chunk {
        size_t prev_foot;
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
        Segment* next;
    };
    Buddy& _buddy;
    uint32_t _freeMap;
    Chunk* _top;
    Chunk* _free[16];
    Segment _head;
};

} /* namespace memory */
#endif /* ifndef __MEMORY_DL_MALLOC_H__ */
