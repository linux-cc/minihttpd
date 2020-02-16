#include "util/config.h"
#include "memory/slab_malloc.h"
#include "memory/buddy_malloc.h"
#include <stdio.h>

#define ALIGN8_SIZE         8
#define ALIGN8_MASK         7
#define ALIGN8_SHIFT        3
#define ALIGN8(n)           (((n) + ALIGN8_MASK) & ~ALIGN8_MASK)
#define SLAB_MAX_SIZE       (MAX_FREE_NUM << ALIGN8_SHIFT)
#define SLAB_FREE_IDX(n)    (((n) >> ALIGN8_SHIFT) - 1)
#define PAGE_SIZE           4096

namespace memory {

SlabMalloc::SlabMalloc(BuddyMalloc &buddy): _buddy(buddy) {
    for (int i = 0; i < MAX_FREE_NUM; ++i) {
        _free[i] = NULL;
    }
}

void *SlabMalloc::alloc(int size) {
    int alignSize = ALIGN8(size);
    if (alignSize > SLAB_MAX_SIZE) {
        return NULL;
    }
    int idx = SLAB_FREE_IDX(alignSize);
    if (!_free[idx] || !_free[idx]->free) {
        void *page = _buddy.alloc(PAGE_SIZE);
        if (!page) return NULL;
        initPage(idx, page);
    }
    SlabInfo *slab = _free[idx];
    Chunk *chunk = slab->free;
    slab->free = chunk->next;
    ++slab->allocated;
    if (!slab->free) {
        _free[idx] = slab->next;
    }
    _LOG_("slab: {idx: %d, this: %p, prev: %p, next: %p, size: %03d, allocated: %03d, free: %p}, size: %d, alloc: %p\n",
          idx, slab, slab->prev, slab->next, slab->chunkSize, slab->allocated, slab->free, size, chunk);

    return chunk;
}

void SlabMalloc::initPage(int idx, void *page) {
    SlabInfo *slab = (SlabInfo*)page;
    slab->allocated = 0;
    slab->chunkSize = (idx + 1) << ALIGN8_SHIFT;
    slab->free = (Chunk*)(slab + 1);
    int n = (PAGE_SIZE - sizeof(SlabInfo)) / slab->chunkSize;
    Chunk *chunk = slab->free;
    for (int i = 0; i < n - 1; ++i) {
        chunk->next = (Chunk*)((char*)chunk + slab->chunkSize);
        chunk = chunk->next;
    }
    chunk->next = NULL;
    linkSlab(idx, slab);
}

void SlabMalloc::linkSlab(int idx, SlabInfo *slab) {
    SlabInfo *header = _free[idx];
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

void SlabMalloc::free(const void* addr) {
    SlabInfo* slab = (SlabInfo*)((intptr_t)addr & ~(PAGE_SIZE - 1));
    bool wasEmpty = !slab->free;
    Chunk* chunk = (Chunk*)addr;
    chunk->next = slab->free;
    slab->free = chunk;
    --slab->allocated;
    _LOG_("slab: {idx: %d, this: %p, prev: %p, next: %p, size: %03d, allocated: %03d, free: %p}, addr: %p, wasEmpty: %d\n",
          SLAB_FREE_IDX(slab->chunkSize), slab, slab->prev, slab->next, slab->chunkSize, slab->allocated, slab->free, addr, wasEmpty);
    if (wasEmpty || !slab->allocated) {
        SlabInfo *next = slab->next, *prev = slab->prev;
        prev->next = next;
        next->prev = prev;
        int idx = SLAB_FREE_IDX(slab->chunkSize);
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

char* SlabMalloc::dump() {
    char* buf = new char[4096];
    int pos = 0;
    for (int i = 0; i < MAX_FREE_NUM; ++i) {
        SlabInfo* slab = _free[i];
        if (slab) {
            pos += sprintf(buf + pos, "[%02d][%p]prev: %p, next: %p, chunksize: %03d, allocated: %03d, free: %p\n",
                           i, slab, slab->prev, slab->next, slab->chunkSize, slab->allocated, slab->free);
            SlabInfo* next = slab->next;
            while (next != slab) {
                pos += sprintf(buf + pos, "[%02d][%p]prev: %p, next: %p, chunksize: %03d, allocated: %03d, free: %p\n",
                               i, next, next->prev, next->next, next->chunkSize, next->allocated, next->free);
                next = next->next;
            }
        }
    }
    return buf;
}

} /* namespace memory */
