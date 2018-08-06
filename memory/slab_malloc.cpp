#include "memory/slab_malloc.h"
#include <stdio.h>

#define SLAB_MAX_SIZE       (MAX_FREE_NUM << ALIGN8_SHIFT)
#define SLAB_FREE_IDX(n)    (((n) >> ALIGN8_SHIFT) - 1)

BEGIN_NS(memory)

SlabMalloc::SlabMalloc(Buddy &buddy):
_buddy(buddy) {
    for (int i = 0; i < MAX_FREE_NUM; ++i) {
        _free[i] = NULL;
    }
}

void *SlabMalloc::alloc(size_t size) {
    size = ALIGN8(size);
    if (size > SLAB_MAX_SIZE) {
        return NULL;
    }
    size_t idx = SLAB_FREE_IDX(size);
    if (!_free[idx] || !_free[idx]->free) {
        void *page = _buddy.alloc(1);
        if (!page) {
            return NULL;
        }
        initPage(idx, page);
    }
    Info *slab = _free[idx];
    Chunk *chunk = slab->free;
    slab->free = chunk->next;
    ++slab->allocated;
    if (!slab->free) {
        _free[idx] = slab->next;
    }

    return chunk;
}

void SlabMalloc::initPage(size_t idx, void *page) {
    Info *slab = (Info*)page;
    slab->allocated = 0;
    slab->chunkSize = (idx + 1) << ALIGN8_SHIFT;
    slab->free = (Chunk*)(slab + 1);
    size_t n = (PAGE_SIZE - sizeof(Info)) / slab->chunkSize;
    Chunk *chunk = slab->free;
    for (size_t i = 0; i < n - 1; ++i) {
        chunk->next = (Chunk*)((char*)chunk + slab->chunkSize);
        chunk = chunk->next;
    }
    chunk->next = NULL;
    linkSlab(idx, slab);
}

void SlabMalloc::linkSlab(size_t idx, Info *slab) {
    Info *header = _free[idx];
    if (!header) {
        slab->next = slab;
        slab->prev = slab;
    } else {
        slab->prev = header->prev;
        slab->prev->next = slab;
        slab->next = header;
        slab->next->prev = slab;
    }
    _free[idx] = slab;
}

void SlabMalloc::free(void *addr) {
    Info *slab = (Info*)((intptr_t)addr & ~PAGE_MASK);
    bool wasEmpty = !slab->free;
    Chunk *chunk = (Chunk*)addr;
    chunk->next = slab->free;
    slab->free = chunk;
    --slab->allocated;
    
    if (wasEmpty || !slab->allocated) {
        Info *next = slab->next, *prev = slab->prev;
        prev->next = next;
        next->prev = prev;
        size_t idx = SLAB_FREE_IDX(slab->chunkSize);
        if (_free[idx] == slab) {
            _free[idx] = next == slab ? NULL : next;
        }
        if (wasEmpty) {
            linkSlab(idx, slab);
        } else {
            _buddy.free(slab);
        }
    }
}

char *SlabMalloc::dump() {
    char *buf = new char[4096];
    int pos = 0;
    for (int i = 0; i < MAX_FREE_NUM; ++i) {
        Info *slab = _free[i];
        if (slab) {
            int chunksize = (i + 1) * ALIGN8_SIZE;
            pos += sprintf(buf + pos, "[%d][%p]free: %p\n", chunksize, slab, slab->free);
            Info *next = slab->next;
            while (next != slab) {
                pos += sprintf(buf + pos, "[%d][%p]free: %p\n", chunksize, next, next->free);
                next = next->next;
            }
        }
    }
    return buf;
}

END_NS
