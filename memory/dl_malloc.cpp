#include "memory/dl_malloc.h"
#include "memory/buddy.h"
#include <stdio.h>

namespace memory {

DlMalloc::DlMalloc(Buddy& buddy):
_buddy(buddy),
_top(NULL),
_head(NULL),
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
            initTop(base, size - footSize());
            _head = (Segment*)chunk2mem(nextChunk(_top));
            _head->base = (char*)base;
            _head->size = size;
            _head->prev = _head->next = _head;
        } else {
            mergeSeg(base, size);
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

void DlMalloc::mergeSeg(void* base, size_t size) {
    Segment* near = getNear(base, size, true);
    if (near) {
        Segment* near2 = getNear(base, size, false);
        appendSeg(near, near2, base, size);
    } else {
        near = getNear(base, size, false);
        if (near) {
            prependSeg(near, base, size);
        } else {
            addSegment(base, size);
        }
    }
}

DlMalloc::Segment* DlMalloc::getNear(void* base, size_t size, bool append) {
    Segment* ps = _head;
    char *pc = (char*)base;
    do {
        bool find = append ? (pc == ps->base + ps->size) : (pc + size == ps->base);
        if (find) {
            return ps;
        }
        ps = ps->next;
    } while (ps != _head);

    return NULL;
}

void DlMalloc::appendSeg(Segment* near1, Segment* near2, void* base, size_t size) {
    Segment* curr = (Segment*)chunk2mem(nextChunk(base, size - footSize()));
    curr->base = near1->base;
    curr->size = near1->size + size;
    Chunk* chunk = _top;
    bool insertTop = false;
    if (holdsTop(near1)) {
        size += topSize();
    } else {
        chunk = mem2chunk(near1);
        if (!bitPSet(chunk)) {
            size += chunk->prevSize;
            chunk = prevChunk(chunk);
            unlinkChunk(chunk);
        }
        insertTop = true;
    }

    if (near2) {
        Chunk* next = (Chunk*)near2->base;
        near2->base = curr->base;
        near2->size += curr->size;
        curr = near2;
        if (next == _top) {
            if (insertTop) {
                insertTop = false;
            }
        } else if (!bitCSet(next)) {
            size += chunkSize(next);
            next = nextChunk(next);
        }
        size += footSize();
        setChunkFree(chunk, size, next);
    }
    if (insertTop) {
        insertChunk(_top, topSize());
    }
    setTop(chunk, size);
    curr->prev = near1->prev;
    curr->prev->next = curr;
    curr->next = near1->next;
    curr->next->prev = curr;
}

void DlMalloc::prependSeg(Segment* near, void* base, size_t size) {
    char* oldBase = near->base;
    near->base = (char*)base;
    near->size += size;
    Chunk* chunk = (Chunk*)base;
    Chunk* next = (Chunk*)oldBase;
    if (next == _top) {
        setTop(chunk, topSize() + size);
    } else {
        insertChunk(_top, topSize());
        if (!bitCSet(next)) {
            size_t oldSize = chunkSize(next);
            unlinkChunk(next);
            next = nextChunk(next, oldSize);
            size += oldSize;
        }
        setChunkFree(chunk, size, next);
        _top = chunk;
    }
}

void DlMalloc::addSegment(void* base, size_t size) {
    Chunk* oldTop = _top;
    size_t oldSize = topSize();
    initTop(base, size - footSize());
    Segment* seg = (Segment*)chunk2mem(nextChunk(_top));
    seg->base = (char*)base;
    seg->size = size;
    seg->prev = _head->prev;
    seg->prev->next = seg;
    seg->next = _head;
    seg->next->prev = seg;
    _head = seg;
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
    Chunk* chunk = mem2chunk(addr);
    size_t size = chunkSize(chunk);
    Chunk* next = nextChunk(chunk, size);
    Chunk* prev = chunk;
    if (!bitPSet(chunk)) {
        size += chunk->prevSize;
        prev = prevChunk(chunk);
        if (prev != _top) {
            unlinkChunk(prev);
        }
    }

    if (!bitCSet(next)) {
        size += chunkSize(next);
        if (next == _top) {
            setTop(prev, size);
            return;
        } else {
            unlinkChunk(next);
            if (prev != _top) {
                setChunkFree(prev, size);
            } else {
                setTop(prev, size);
                return;
            }
        }
    } else {
        setChunkFree(prev, size, next);
    }

    insertChunk(prev, size);
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
    Segment* s = _head;
    do {
        pos += sprintf(buf + pos, "segment[%p] base:%p, size:%lu, prev: %p, next:%p\n",
            s, s->base, s->size, s->prev, s->next);
        s = s->next;
    } while (s != _head);
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
