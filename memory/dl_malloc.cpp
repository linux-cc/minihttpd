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
        Chunk* chunk = getFitChunk(nb, idx, rsize);
        if (chunk) {
            unlinkChunk(chunk);
            if (rsize < minChunkSize()) {
                setChunkUsedWithP(chunk, rsize + nb);
            } else {
                Chunk* next = nextChunk(chunk, nb);
                setChunkUsed(chunk, nb);
                setChunkFree(next, rsize);
                insertChunk(next, rsize);
            }
            return chunk2mem(chunk);
        }
    }
    size_t tsize = topSize();
    if (nb < tsize) {
        Chunk* oldTop = _top;
        setTop(nextChunk(oldTop, nb), tsize - nb);
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
                clearTreeMap(chunk->index);
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
    if (!treeMapIsMark(i)) {
        markTreeMap(i);
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
    if (base) {
        size_t size = (nb + _buddy.pageMask()) & ~_buddy.pageMask();
        if (!_top) {
            _head.base = (char*)base;
            _head.size = size;
            _head.next = NULL;
            initTop(base, size - footSize());
        } else {
            mergeSeg(base, size, nb);
        }
        size_t tsize = topSize();
        if (nb < tsize) {
            Chunk* oldTop = _top;
            setTop(nextChunk(oldTop, nb), tsize - nb);
            setChunkUsed(oldTop, nb);
            return chunk2mem(oldTop);
        }
    }

    return NULL;
}

void DlMalloc::mergeSeg(void* base, size_t size, size_t nb) {
    Segment* sp = &_head, *prevSp = NULL;
    char* newBase = (char*)base;
    while (sp && newBase != sp->base + sp->size) {
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
        while (sp && sp->base != newBase + size) {
            sp = sp->next;
        }
        if (sp) {
            char* oldBase = sp->base;
            sp->base = newBase;
            sp->size += size;
            prependSeg(newBase, oldBase, nb);
        } else {
            addSegment(newBase, size);
        }
    }
}

void DlMalloc::appendSeg(Segment* prev, Segment* sp, void* base, size_t size) {
    size_t fsize = footSize();
    Chunk* chunk = nextChunk(sp->base, sp->size - fsize);
    Segment* curr = (Segment*)chunk2mem(nextChunk(base, size - fsize));
    *curr = *(Segment*)chunk2mem(chunk);
    curr->size += size;
    prev->next = curr;
    insertChunk(_top, topSize());
    if (!bitPSet(chunk)) {
        size += chunk->prevSize;
        chunk = prevChunk(chunk);
        unlinkChunk(chunk);
    }
    initTop(chunk, size);
}

void DlMalloc::prependSeg(void* newBase, void* oldBase, size_t nb) {
    Chunk* pNew = (Chunk*)newBase;
    Chunk* pOld = (Chunk*)oldBase;
    size_t size = (char*)pOld - (char*)pNew;
    if (pOld == _top) {
        setTop(pNew, topSize() + size);
    } else {
        insertChunk(_top, topSize());
        if (!bitCSet(pOld)) {
            size_t oldSize = chunkSize(pOld);
            unlinkChunk(pOld);
            pOld = nextChunk(pOld, oldSize);
            size += oldSize;
        }
        setChunkFree(pNew, size, pOld);
        _top = pNew;
    }
}

void DlMalloc::addSegment(void* base, size_t size) {
    Chunk* oldTop = _top;
    size_t oldSize = topSize();
    Segment* seg = (Segment*)chunk2mem(nextChunk(_top));

    initTop(base, size - footSize());
    *seg = _head;
    _head.base = (char*)base;
    _head.size = size;
    _head.next = seg;
    Chunk* n = nextChunk(oldTop, oldSize);
    setChunkFree(oldTop, oldSize, n);
    insertChunk(oldTop, oldSize);
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
    Chunk* chunk = mem2chunk(addr);
    size_t size = chunkSize(chunk);
    Chunk* next = nextChunk(chunk, size);
    if (!bitPSet(chunk)) {
        size += chunk->prevSize;
        chunk = prevChunk(chunk);
        unlinkChunk(chunk);
    }

    if (!bitCSet(next)) {
        if (next == _top) {
            setTop(chunk, topSize() + size);
            return;
        } else {
            size += chunkSize(next);
            unlinkChunk(next);
            setChunkFree(chunk, size);
        }
    } else {
        setChunkFree(chunk, size, next);
    }

    insertChunk(chunk, size);
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
