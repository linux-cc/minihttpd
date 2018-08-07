#include "memory/dl_malloc.h"
#include <stdio.h>

#define SEGMENT_PAGES               1
#define TREEBIN_SHIFT               7
#define SIZE_T_ZERO                 0
#define SIZE_T_ONE                  1
#define SIZE_T_TWO                  2
#define SIZE_T_FOUR                 4 
#define SIZE_T_SIZE                 (sizeof(size_t))
#define SIZE_T_BITSIZE              (SIZE_T_SIZE << 3)
#define TWO_SIZE_T_SIZES            (SIZE_T_SIZE << 1)
#define MALLOC_ALIGNMENT            (2 * sizeof(void *))
#define CHUNK_ALIGN_MASK            (MALLOC_ALIGNMENT - SIZE_T_ONE)
#define PAD_REQUEST(n)              (((n) + SIZE_T_SIZE + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)
#define MIN_CHUNK_SIZE              ((sizeof(Chunk) + CHUNK_ALIGN_MASK) & ~CHUNK_ALIGN_MASK)
#define LSH_FOR_TREE_IDX(i)         ((SIZE_T_BITSIZE-SIZE_T_ONE) - (((i) >> 1) + TREEBIN_SHIFT - 2))
#define P_BIT                       (SIZE_T_ONE)
#define C_BIT                       (SIZE_T_TWO)
#define R_BIT                       (SIZE_T_FOUR)
#define CP_BITS                     (P_BIT|C_BIT)
#define RCP_BITS                    (R_BIT|P_BIT|C_BIT)
#define CHUNK_SIZE(p)               ((p)->head & ~(RCP_BITS))
#define NEXT_CHUNK(p)               ((Chunk*)( ((char*)(p)) + CHUNK_SIZE(p) ))
#define NEXT_CHUNK2(p, s)           ((Chunk*)( ((char*)(p)) + (s) ))
#define IDX2BIT(i)                  (1 << (i))
#define LEFT_BITS(x)                ((x << 1) | -(x << 1))
#define LEAST_BIT(x)                ((x) & -(x))
#define COMPUTE_BIT2IDX(x)          __builtin_ctz(x)
#define LEFTMOST_CHILD(t)           ((t)->child[0] ? (t)->child[0] : (t)->child[1])
#define RIGHTMOST_CHILD(t)          ((t)->child[1] ? (t)->child[1] : (t)->child[0])
#define CHUNK_PLUS_OFFSET(p, s)     ((Chunk*)(((char*)(p)) + (s)))
#define CHUNK_MINUS_OFFSET(p, s)    ((Chunk*)(((char*)(p)) - (s)))
#define CLEAR_FREEMAP(i)            (_freeMap  &= ~IDX2BIT(i))
#define GET_FOOT(p, s)              (NEXT_CHUNK2(p, s)->prev_foot)
#define SET_FOOT(p, s)              (NEXT_CHUNK2(p, s)->prev_foot = (s))
#define SET_CPP_BITS(p, s)          ((p)->head = ((s)|CP_BITS), NEXT_CHUNK2(p, s)->head |= P_BIT)
#define SET_SIZE_P_FOOT(p, s)       ((p)->head = ((s)|P_BIT), SET_FOOT(p, s))
#define SET_SIZE_CP(p, s)           ((p)->head = ((s)|CP_BITS))
#define CLEAR_P_BIT(p)              ((p)->head &= ~P_BIT)
#define SET_FREE_WITH_P(p, s, n)    (CLEAR_P_BIT(n), SET_SIZE_P_FOOT(p, s))
#define CHUNK2MEM(p)                ((void*)((char*)(p) + TWO_SIZE_T_SIZES))
#define MEM2CHUNK(p)                ((Chunk*)((char*)(p) - TWO_SIZE_T_SIZES))
#define MARK_FREEMAP(i)             (_freeMap |=  IDX2BIT(i))
#define CLEAR_FREEMAP(i)            (_freeMap &= ~IDX2BIT(i))
#define FREEMAP_IS_MARKED(i)        (_freeMap &   IDX2BIT(i))
#define SEGMENT_HOLDS(s, t)         ((char*)(t) >= s->base && (char*)(t) < s->base + s->size)
#define C_INUSE(p)                  ((p)->head & C_BIT)
#define P_INUSE(p)                  ((p)->head & P_BIT)
#define INIT_TOP(p, s)              _top = (Chunk*)(p); _topSize = (s); _top->head = (_topSize | P_BIT);\
                                    CHUNK_PLUS_OFFSET((p), (s))->head = TOP_FOOT_SIZE
#define TOP_FOOT_SIZE               (PAD_REQUEST(sizeof(Segment))+MIN_CHUNK_SIZE)
#define PAGES(n)                    (((n) + PAGE_MASK) / PAGE_SIZE)

BEGIN_NS(memory)

DlMalloc::DlMalloc(Buddy &buddy):
_buddy(buddy),
_freeMap(0),
_top(NULL),
_topSize(0)
{
    for (int i = 0; i < MAX_FREE_NUM; ++i) {
        _free[i] = NULL;
    }
}

void *DlMalloc::alloc(size_t size) {
    size_t nb = PAD_REQUEST(size);
    //printf("requst size:%u, aligin:%u\n", size, nb);
    if (_freeMap) {
        size_t rsize = -nb;
        uint32_t idx = computeFreeIndex(nb);
        //printf("chunk index:%u\n", idx);
        Chunk *v = getFitChunk(nb, idx, rsize);
        //printf("getFitChunk:%p\n", v);
        if (v) {
            unlinkChunk(v);
            if (rsize < MIN_CHUNK_SIZE) {
                SET_CPP_BITS(v, rsize + nb);
            } else {
                Chunk *r = CHUNK_PLUS_OFFSET(v, nb);
                SET_SIZE_CP(v, nb);
                SET_SIZE_P_FOOT(r, rsize);
                insertChunk(r, rsize);
            }
            return CHUNK2MEM(v);
        }
    }
    if (nb < _topSize) {
        size_t rsize = _topSize -= nb;
        Chunk *oldTop = _top;
        Chunk *newTop = _top = CHUNK_PLUS_OFFSET(oldTop, nb);
        newTop->head = rsize | P_BIT;
        SET_SIZE_CP(oldTop, nb);
        return CHUNK2MEM(oldTop);
    }
    return sysAlloc(nb);
}

size_t DlMalloc::computeFreeIndex(size_t nb) {
    uint32_t x = nb >> TREEBIN_SHIFT;
    if (x == 0)
        return 0;
    uint32_t k = sizeof(x) * __CHAR_BIT__ - 1 - __builtin_clz(x);
    return (k << 1) + (nb >> (k + TREEBIN_SHIFT - 1) & 1);
}

DlMalloc::Chunk *DlMalloc::getFitChunk(size_t nb, size_t idx, size_t &rsize) {
    Chunk *v = NULL, *t = _free[idx];
    //printf("chunk:%p\n", t);
    if (t) {
        size_t sizebits = nb << LSH_FOR_TREE_IDX(idx);
        //printf("sizebits:%u\n", sizebits);
        Chunk *rst = NULL;
        for (;;) {
            size_t trem = CHUNK_SIZE(t) - nb;
            //printf("trem:%u\n", trem);
            if (trem < rsize) {
                v = t;
                if ((rsize = trem) == 0)
                    break;
            }
            Chunk *rt = t->child[1];
            //printf("child:%u\n", (sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1);
            t = t->child[(sizebits >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1];
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
        uint32_t leftbits = LEFT_BITS(IDX2BIT(idx)) & _freeMap;
        //printf("leftbits:%x, %u\n", LEFT_BITS(IDX2BIT(idx)), leftbits);
        if (leftbits) {
            uint32_t leastbit = LEAST_BIT(leftbits);
            uint32_t i = COMPUTE_BIT2IDX(leastbit);
            t = _free[i];
            //printf("leastbit:%u, index:%u, chunk:%p\n", leastbit, i, t);
        }
    }
    while (t) {
        size_t trem = CHUNK_SIZE(t) - nb;
        //printf("chunk:%p, trem:%u\n", t, trem);
        if (trem < rsize) {
            rsize = trem;
            v = t;
        }
        //printf("child[%u]:", t->child[0] ? 0:1);
        t = LEFTMOST_CHILD(t);
        //printf("%p\n", t);
    }
    return v;
}

void DlMalloc::unlinkChunk(Chunk *chunk) {
    Chunk *cp = chunk->parent;
    Chunk *hold;
    //printf("unlink chunk:%p, next:%p, parent:%p\n", chunk, chunk->next, cp);
    if (chunk->next != chunk) {
        Chunk *next = chunk->next;
        hold = chunk->prev;
        if (next->prev == chunk && hold->next == chunk) {
            next->prev = hold;
            hold->next = next;
        }
    } else {
        Chunk **rmcp = &RIGHTMOST_CHILD(chunk);
        //printf("right most child:%p,%p\n", rmcp, *rmcp);
        if ((hold = *rmcp)) {
            Chunk **rmcr;
            while (*(rmcr = &RIGHTMOST_CHILD(hold))) {
                //printf("%p right most child:%p,%p\n", hold, rmcr, *rmcr);
                hold = *(rmcp = rmcr);
            }
            *rmcp = NULL;
        }
    }
    if (cp) {
        Chunk **head = &_free[chunk->index];
        //printf("chunk head:%p, index:%u, hold:%p\n", *head, chunk->index, hold);
        if (chunk == *head) {
            //printf("chunk == *head\n");
            if (!(*head = hold)) {
                //printf("clean map index:%u\n", chunk->index);
                CLEAR_FREEMAP(chunk->index);
            } 
        } else {
            //printf("chunk != *head\n");
            int child = cp->child[0] == chunk ? 0 : 1;
            cp->child[child] = hold;
        }
        if (hold) {
            Chunk *c0, *c1;
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

void DlMalloc::insertChunk(Chunk *chunk, size_t nb) {
    uint32_t i = computeFreeIndex(nb);
    Chunk **head = &_free[i];
    chunk->index = i;
    chunk->child[0] = chunk->child[1] = NULL;
    if (!FREEMAP_IS_MARKED(i)) {
        MARK_FREEMAP(i);
        *head = chunk;
        chunk->parent = (Chunk*)head;
        chunk->next = chunk->prev = chunk;
    } else {
        Chunk *t = *head;
        size_t k = nb << LSH_FOR_TREE_IDX(i);
        for (;;) {
            if (CHUNK_SIZE(t) != nb) {
                Chunk **c = &(t->child[(k >> (SIZE_T_BITSIZE-SIZE_T_ONE)) & 1]);
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
                Chunk *next = t->next;
                t->next = next->prev = chunk;
                chunk->next = next;
                chunk->prev = t;
                chunk->parent = NULL;
                break;
            }
        }
    }
}

void *DlMalloc::sysAlloc(size_t nb) {
    Segment *sp = _top ? getTopSeg() : NULL;
    void *base;
    if (!sp) {
        base = _buddy.alloc(SEGMENT_PAGES);
        ////printf("buddy allloc:%p\n", base);
        if (!base)
            return NULL;
    }
    size_t size = SEGMENT_PAGES * PAGE_SIZE - TOP_FOOT_SIZE;
    if (!_top) {
        _seg.base = (char*)base;
        _seg.size = size; 
        _seg.next = NULL;
        INIT_TOP(base, size);
        //printf("init _top:%p, size:%u\n", base, size);
    } else {
        void *mem = mergeSeg(base, size, nb);
        if (mem)
            return mem;
    }
    if (nb < _topSize) {
        size_t rsize = _topSize -= nb;
        Chunk *p = _top;
        Chunk *r = _top = CHUNK_PLUS_OFFSET(p, nb);
        r->head = rsize | P_BIT;
        SET_SIZE_CP(p, nb);
        return CHUNK2MEM(p);
    }
    return NULL;
}

DlMalloc::Segment *DlMalloc::getTopSeg() {
    Segment *sp = &_seg;
    char *addr = (char*)_top;
    for (;;) {
        if (addr >= sp->base && addr < sp->base + sp->size)
            return sp;
        if (!(sp = sp->next))
            return NULL;
    }
}

void *DlMalloc::mergeSeg(void *base, size_t size, size_t nb) {
    Segment *sp = &_seg;
    char *cbase = (char*)base;
    while (sp && cbase != sp->base + sp->size)
        sp = sp->next;
    if (sp && SEGMENT_HOLDS(sp, _top)) {
        sp->size += size;
        INIT_TOP(_top, _topSize + size);
    } else {
        sp = &_seg;
        while (sp && sp->base != cbase + size)
            sp = sp->next;
        if (sp) {
            char *oldbase = sp->base;
            sp->base = cbase;
            sp->size += size;
            return prependSeg(cbase, oldbase, nb);
        }
        else
            addSegment(cbase, size);
    }
    return NULL;
}

void *DlMalloc::prependSeg(void *newBase, void *oldBase, size_t nb) {
    Chunk *pUse = (Chunk*)newBase;
    Chunk *pOld = (Chunk*)oldBase;
    size_t size = (char*)pOld - (char*)pUse;
    Chunk *pNew = CHUNK_PLUS_OFFSET(pUse, nb);
    size_t newSize = size - nb;
    SET_SIZE_CP(pUse, nb);

    if (pOld == _top) {
        _topSize += newSize;
        _top = pNew;
        _top->head = _topSize | P_BIT;
    } else {
        if (!C_INUSE(pOld)) {
            size_t nsize = CHUNK_SIZE(pOld);
            unlinkChunk(pOld);
            pOld = CHUNK_PLUS_OFFSET(pOld, nsize);
            newSize += nsize;
        }
        SET_FREE_WITH_P(pNew, newSize, pOld);
        insertChunk(pNew, newSize);
    }

    return CHUNK2MEM(pUse);
}

void DlMalloc::addSegment(void *base, size_t size) {
    Segment *oldsp = getTopSeg();
    char *oldtop = (char*)_top;
    char *oldend = oldsp->base + oldsp->size;
    Chunk *cp = (Chunk*)oldend;
    Segment *newsp = (Segment*)CHUNK2MEM(cp);

    INIT_TOP(base, size);
    SET_SIZE_CP(cp, PAD_REQUEST(sizeof(Segment)));
    *newsp = _seg;
    _seg.base = (char*)base;
    _seg.size = size;
    _seg.next = newsp;

    if (oldend - MIN_CHUNK_SIZE > oldtop) {
        Chunk *p = (Chunk*)oldtop;
        size_t psize = oldend - oldtop;
        Chunk *n = CHUNK_PLUS_OFFSET(p, psize);
        SET_FREE_WITH_P(p, psize, n);
        insertChunk(p, psize);
    }
}

void DlMalloc::free(void *addr) {
    if (!addr)
        return;
    Chunk *p = MEM2CHUNK(addr);
    size_t psize = CHUNK_SIZE(p);
    Chunk *next = CHUNK_PLUS_OFFSET(p, psize);
    //printf("free chunk:%p, size:%u\n", p, psize);
    if (!P_INUSE(p)) {
        size_t prevsize = p->prev_foot;
        Chunk *prev = CHUNK_MINUS_OFFSET(p, prevsize);
        //printf("merge prev chunk:%p, size:%u\n", prev, prevsize);
        psize += prevsize;
        p = prev;
        unlinkChunk(p);
    }

    if (!C_INUSE(next)) {
        if (next == _top) {
            size_t tsize = _topSize += psize;
            _top = p;
            p->head = tsize | P_BIT;
            return;
        } else {
            size_t nsize = CHUNK_SIZE(next);
            psize += nsize;
            unlinkChunk(next);
            SET_SIZE_P_FOOT(p, psize);
        }
    } else {
        SET_FREE_WITH_P(p, psize, next);
    }

    insertChunk(p, psize);
}

char *DlMalloc::dump() {
    char *buf = new char[4096];
    int pos = 0;
    int bits = sizeof(_freeMap) * __CHAR_BIT__;
    //pos += sprintf(buf + pos, "%s", "_freeMap:");
    for (int i = 0; i < bits; ++i) {
        //int b = (_freeMap >> (bits - 1 - i)) & 1;
        //pos += sprintf(buf + pos, "%d", b);
    }
    //pos += sprintf(buf + pos, "%s", "\n");
    if (_top) {
        //pos += sprintf(buf + pos, "_top:%p, size:%u, head:%u, C:%u, P:%u, prev:%p, next:%p\n", 
        //        _top, _topSize, _top->head, C_INUSE(_top) ? 1:0, P_INUSE(_top) ? 1:0, _top->prev, _top->next);
    }
    //pos += sprintf(buf + pos, "segment base:%p, size:%u, next:%p\n", _seg.base, _seg.size, _seg.next);
    Segment *s = _seg.next;
    while (s) {
        //pos += sprintf(buf + pos, "segment base:%p, size:%u, next:%p\n", s->base, s->size, s->next);
        s = s->next;
    }
    dumpChunk(buf + pos);
    return buf;
}

void DlMalloc::dumpChunk(char *buf) {
    int pos = 0;
    for (int i = 0; i < MAX_FREE_NUM; ++i) {
        pos += sprintf(buf + pos, "free list[%d]:", i);
        pos += dumpChunk(buf + pos, _free[i]);
        buf[pos++] = '\n';
    }
}

int DlMalloc::dumpChunk(char *buf, Chunk *chunk) {
    if (!chunk)
        return 0;
    Chunk *cp = chunk->parent;
    int pos = 0;
    if (cp) {
        pos += sprintf(buf + pos, "%p:%u,%d", chunk, chunk->index, chunk == cp->child[0] ? 0 : 1);
        pos += dumpChunk(buf + pos, chunk->child[0]);
        pos += dumpChunk(buf + pos, chunk->child[1]);
    }
    return pos;
}

END_NS
