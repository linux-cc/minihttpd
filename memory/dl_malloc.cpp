#include "memory/dl_malloc.h"
#include "memory/buddy.h"
#include <stdio.h>

namespace memory {

DlMalloc::DlMalloc(Buddy& buddy):
_buddy(buddy),
_top(NULL),
_treeMap(0)
{
    for (int i = 0; i < TREE_NUM; ++i) {
        _tree[i] = NULL;
    }
}

void* DlMalloc::alloc(size_t size) {
    size_t nb = padRequest(size);
    if (_treeMap) {
        size_t rsize = -nb;
        uint32_t idx = computeTreeIndex(nb);
        Chunk* v = getFitChunk(nb, idx, rsize);
        if (v) {
            unlinkChunk(v);
            if (rsize < minChunkSize()) {
                setChunkUsedWithP(v, rsize + nb);
            } else {
                Chunk* r = chunkPlusOffset(v, nb);
                setChunkUsed(v, nb);
                setChunkFreeWithP(r, rsize);
                insertChunk(r, rsize);
            }
            return chunk2mem(v);
        }
    }
    size_t tsize = topSize();
    if (nb < tsize) {
        Chunk* oldTop = _top;
        setTop(chunkPlusOffset(oldTop, nb), tsize - nb);
        setChunkUsed(oldTop, nb);
        return chunk2mem(oldTop);
    }
    return sysAlloc(nb);
}

uint32_t DlMalloc::computeTreeIndex(size_t nb) {
    uint32_t x = nb >> TREE_SHIFT;
    if (x == 0)
        return 0;
    uint32_t k = sizeof(x) *  __CHAR_BIT__ - 1 - __builtin_clz(x);
    return (k << 1) + ((nb >> (k + TREE_SHIFT - 1)) & 1);
}

DlMalloc::Chunk* DlMalloc::getFitChunk(size_t nb, size_t idx, size_t &rsize) {
    Chunk* v = NULL, *t = _tree[idx];
    if (t) {
        size_t sizebits = nb << lshForTreeIdx(idx);
        Chunk* rst = NULL;
        for (;;) {
            size_t trem = chunkSize(t) - nb;
            if (trem < rsize) {
                v = t;
                if ((rsize = trem) == 0)
                    break;
            }
            Chunk* rt = t->child[1];
            t = t->child[(sizebits >> SIZE_T_BITSIZE) & 1];
            if (rt && rt != t)
                rst = rt;
            if (!t) {
                t = rst;
                break;
            }
            sizebits <<= 1;
        }
    }
    if (!t && !v) {
        uint32_t leftBits = getLeftBits(idx);
        if (leftBits) {
            uint32_t leastBit = getLeastBit(leftBits);
            uint32_t i = __builtin_ctz(leastBit);
            t = _tree[i];
        }
    }
    while (t) {
        size_t trem = chunkSize(t) - nb;
        if (trem < rsize) {
            rsize = trem;
            v = t;
        }
        t = leftMostChild(t);
    }
    return v;
}

void DlMalloc::unlinkChunk(Chunk* chunk) {
    Chunk* cp = chunk->parent;
    Chunk* hold;
    if (chunk->next != chunk) {
        Chunk* next = chunk->next;
        hold = chunk->prev;
        if (next->prev == chunk && hold->next == chunk) {
            next->prev = hold;
            hold->next = next;
        }
    } else {
        Chunk** rmcp = &rightMostChild(chunk);
        if ((hold = *rmcp)) {
            Chunk** rmcr;
            while (*(rmcr = &rightMostChild(hold))) {
                hold = *(rmcp = rmcr);
            }
            *rmcp = NULL;
        }
    }
    if (cp) {
        Chunk** head = &_tree[chunk->index];
        if (chunk == *head) {
            if (!(*head = hold)) {
                clearBitMap(chunk->index);
            } 
        } else {
            int child = cp->child[0] == chunk ? 0 : 1;
            cp->child[child] = hold;
        }
        if (hold) {
            Chunk* c0, *c1;
            hold->parent = cp;
            if ((c0 = chunk->child[0])) {
                hold->child[0] = c0;
                c0->parent = hold;
            }
            if ((c1 = chunk->child[1])) {
                hold->child[1] = c1;
                c1->parent = hold;
            }
        }
    }
}

void DlMalloc::insertChunk(Chunk* chunk, size_t nb) {
    uint32_t i = computeTreeIndex(nb);
    Chunk** head = &_tree[i];
    chunk->index = i;
    chunk->child[0] = chunk->child[1] = NULL;
    if (!bitMapIsMark(i)) {
        markBitMap(i);
        *head = chunk;
        chunk->parent = (Chunk*)head;
        chunk->next = chunk->prev = chunk;
    } else {
        Chunk* t = *head;
        size_t k = nb << lshForTreeIdx(i);
        for (;;) {
            if (chunkSize(t) != nb) {
                Chunk** c = &(t->child[(k >> SIZE_T_BITSIZE) & 1]);
                k <<= 1;
                if (*c) {
                    t = *c;
                } else {
                    *c = chunk;
                    chunk->parent = t;
                    chunk->next = chunk->prev = chunk;
                    break;
                }
            } else {
                Chunk* next = t->next;
                t->next = next->prev = chunk;
                chunk->next = next;
                chunk->prev = t;
                chunk->parent = NULL;
                break;
            }
        }
    }
}

void* DlMalloc::sysAlloc(size_t nb) {
    void* base = _buddy.alloc(nb);
    size_t size = (nb + _buddy.pageMask()) & ~_buddy.pageMask();
    if (!base) {
        return NULL;
    }
    if (!_top) {
        _head.base = (char*)base;
        _head.size = size; 
        _head.next = NULL;
        initTop(base, size - footSize());
    } else {
        void* mem = mergeSeg(base, size, nb);
        if (mem) {
            return mem;
        }
    }
    size_t tsize = topSize();
    if (nb < tsize) {
        Chunk* oldTop = _top;
        setTop(chunkPlusOffset(oldTop, nb), tsize - nb);
        setChunkUsed(oldTop, nb);
        return chunk2mem(oldTop);
    }
    return NULL;
}

void* DlMalloc::mergeSeg(void* base, size_t size, size_t nb) {
    Segment* sp = &_head, *prevSp = NULL;
    char* cbase = (char*)base;
    while (sp && cbase != sp->base + sp->size) {
        prevSp = sp;
        sp = sp->next;
    }
    if (sp) {
        if (holdsTop(sp)) {
            initTop(_top, topSize() + size);
            sp->size += size;
        } else {
            appendSeg(prevSp, sp, base, size);
        }
    } else {
        sp = &_head;
        while (sp && sp->base != cbase + size) {
            prevSp = sp;
            sp = sp->next;
        }
        if (sp) {
            char* oldbase = sp->base;
            sp->base = cbase;
            sp->size += size;
            return prependSeg(cbase, oldbase, nb);
        } else {
            addSegment(cbase, size);
        }
    }
    return NULL;
}

void DlMalloc::appendSeg(Segment* prev, Segment* sp, void* base, size_t size) {
    size_t fsize = footSize();
    Chunk* next = chunkPlusOffset(sp->base, sp->size - fsize);
    Chunk* newNext = chunkPlusOffset(base, size - fsize);
    Segment* newSp = (Segment*)chunk2mem(newNext);
    *newSp = *(Segment*)chunk2mem(next);
    newSp->size += size;
    prev->next = newSp;
    insertChunk(_top, topSize());
    if (!bitPSet(next)) {
        size_t psize = next->prevSize;
        Chunk* prev = chunkMinusOffset(next, psize);
        unlinkChunk(prev);
        initTop(prev, psize + size);
    } else {
        initTop(next, size);
    }
}

void* DlMalloc::prependSeg(void* newBase, void* oldBase, size_t nb) {
    Chunk* pNew = (Chunk*)newBase;
    Chunk* pOld = (Chunk*)oldBase;
    Chunk* chunk = chunkPlusOffset(pNew, nb);
    size_t size = (char*)pOld - (char*)pNew - nb;
    setChunkUsed(pNew, nb);
    if (pOld == _top) {
        setTop(chunk, topSize() + size);
    } else {
        if (!bitCSet(pOld)) {
            size_t oldSize = chunkSize(pOld);
            unlinkChunk(pOld);
            pOld = chunkPlusOffset(pOld, oldSize);
            size += oldSize;
        }
        setChunkFree(chunk, size, pOld);
        insertChunk(chunk, size);
    }

    return chunk2mem(pNew);
}

void DlMalloc::addSegment(void* base, size_t size) {
    Chunk* oldTop = _top;
    size_t oldSize = topSize();
    Chunk* cp = nextChunk(_top);
    Segment* seg = (Segment*)chunk2mem(cp);

    initTop(base, size - footSize());
    setChunkUsed(cp, chunkSize(cp));
    *seg = _head;
    _head.base = (char*)base;
    _head.size = size;
    _head.next = seg;
    if (oldSize > minChunkSize()) {
        Chunk* n = chunkPlusOffset(oldTop, oldSize);
        setChunkFree(oldTop, oldSize, n);
        insertChunk(oldTop, oldSize);
    }
}

void DlMalloc::initTop(void* base, size_t size) {
    _top = (Chunk*)base;
    _top->head = size | BIT_P;
    Chunk* next = nextChunk(_top, size);
    next->prevSize = size;
    next->head = footSize() | BIT_C;
}

void DlMalloc::free(void* addr) {
    if (!addr)
        return;
    Chunk* p = mem2chunk(addr);
    size_t psize = chunkSize(p);
    Chunk* next = chunkPlusOffset(p, psize);
    if (!bitPSet(p)) {
        size_t prevsize = p->prevSize;
        Chunk* prev = chunkMinusOffset(p, prevsize);
        psize += prevsize;
        p = prev;
        unlinkChunk(p);
    }

    if (!bitCSet(next)) {
        if (next == _top) {
            setTop(p, topSize() + psize);
            return;
        } else {
            size_t nsize = chunkSize(next);
            psize += nsize;
            unlinkChunk(next);
            setChunkFreeWithP(p, psize);
        }
    } else {
        setChunkFree(p, psize, next);
    }

    insertChunk(p, psize);
}

char* DlMalloc::dump() {
    char* buf = new char[4096];
    int pos = 0;
    int bits = sizeof(_treeMap)*  __CHAR_BIT__;
    pos += sprintf(buf + pos, "%s", "_treeMap:");
    for (int i = 0; i < bits; ++i) {
        int b = (_treeMap >> (bits - 1 - i)) & 1;
        pos += sprintf(buf + pos, "%d", b);
    }
    pos += sprintf(buf + pos, "%s", "\n");
    if (_top) {
        pos += sprintf(buf + pos, "_top:%p, size:%lu, head:%lu, C:%u, P:%u, prev:%p, next:%p\n", 
                _top, topSize(), _top->head, bitCSet(_top) ? 1:0, bitPSet(_top) ? 1:0, _top->prev, _top->next);
    }
    pos += sprintf(buf + pos, "segment[%p] base:%p, size:%lu, next:%p\n", &_head, _head.base, _head.size, _head.next);
    Segment* s = _head.next;
    while (s) {
        pos += sprintf(buf + pos, "segment[%p] base:%p, size:%lu, next:%p\n", s, s->base, s->size, s->next);
        s = s->next;
    }
    dumpChunk(buf + pos);
    return buf;
}

void DlMalloc::dumpChunk(char* buf) {
    int pos = 0;
    for (int i = 0; i < TREE_NUM; ++i) {
        if (_tree[i]) {
            pos += sprintf(buf + pos, "list[%d][%p]:", i, &_tree[i]);
            pos += dumpChunk(buf + pos, _tree[i]);
        }
    }
}

int DlMalloc::dumpChunk(char* buf, Chunk* chunk) {
    if (!chunk) {
        return 0;
    }
    int pos = 0;
    pos += sprintf(buf + pos, "chunk: %p, index: %d, head: %lu, parent:%p, child[0]:%p, child[1]:%p\n",
        chunk, chunk->index, chunk->head, chunk->parent, chunk->child[0], chunk->child[1]);
    Chunk* next = chunk->next;
    while (next != chunk) {
        pos += sprintf(buf + pos, "next chunk: %p, index: %d, head: %lu, parent:%p, child[0]:%p, child[1]:%p\n",
            next, next->index, next->head, next->parent, next->child[0], next->child[1]);
        next = next->next;
    }
    pos += dumpChunk(buf + pos, chunk->child[0]);
    pos += dumpChunk(buf + pos, chunk->child[1]);
    return pos;
}

} /* namespace memory */
