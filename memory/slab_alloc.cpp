#include "util/config.h"
#include "util/util.h"
#include "memory/slab_alloc.h"
#include "memory/buddy_alloc.h"
#include <stdio.h>

#define ALIGN8(n)           (((n) + ALIGN8_MASK) & ~ALIGN8_MASK)
#define SLAB_FREE_IDX(n)    (((n) >> ALIGN8_SHIFT) - 1)

namespace memory {

SlabAlloc::SlabAlloc(BuddyAlloc &buddy): _buddy(buddy) {
    _pageSize = _buddy.getPageSize();
    for (int i = 0; i < SLAB_FREE_NUM; ++i) {
        _free[i] = NULL;
    }
}

void *SlabAlloc::alloc(size_t size) {
    int alignSize = ALIGN8(size);
    if (alignSize > SLAB_MAX_SIZE) {
        return _buddy.alloc(size);
    }
    
    int idx = SLAB_FREE_IDX(alignSize);
    if (!_free[idx] || !_free[idx]->free) {
        void *page = _buddy.alloc(_pageSize);
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
    LOG_DEBUG("slab:{id: %02d, %p <- %p -> %p, alloc: %03d, free: %p}, size: %d, alloc: %p",
          idx, slab->prev, slab, slab->next, slab->allocated, slab->free, size, chunk);

    return chunk;
}

void SlabAlloc::initPage(int idx, void *page) {
    SlabInfo *slab = (SlabInfo*)((char*)page + _pageSize - sizeof(SlabInfo));
    slab->allocated = 0;
    slab->free = (Chunk*)page;
    int chunkSize = (idx + 1) << ALIGN8_SHIFT;
    int n = (_pageSize - sizeof(SlabInfo)) / chunkSize;
    Chunk *chunk = slab->free;
    for (int i = 0; i < n - 1; ++i) {
        chunk->next = (Chunk*)((char*)chunk + chunkSize);
        chunk = chunk->next;
    }
    chunk->next = NULL;
    linkSlab(idx, slab);
}

void SlabAlloc::linkSlab(int idx, SlabInfo *slab) {
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

void SlabAlloc::free(const void* addr, size_t size) {
    int alignSize = ALIGN8(size);
    if (alignSize > SLAB_MAX_SIZE) {
        return _buddy.free(addr);
    }
    
    SlabInfo* slab = (SlabInfo*)(((intptr_t)addr & ~(_pageSize - 1)) + _pageSize - sizeof(SlabInfo));
    bool wasEmpty = !slab->free;
    Chunk* chunk = (Chunk*)addr;
    chunk->next = slab->free;
    slab->free = chunk;
    --slab->allocated;
    LOG_DEBUG("slab: {idx: %02d, %p <- %p -> %p, alloc: %03d, free: %p}, addr: %p, wasEmpty: %d",
          SLAB_FREE_IDX(alignSize), slab->prev, slab, slab->next, slab->allocated, slab->free, addr, wasEmpty);
    if (wasEmpty || !slab->allocated) {
        SlabInfo *next = slab->next, *prev = slab->prev;
        prev->next = next;
        next->prev = prev;
        int idx = SLAB_FREE_IDX(alignSize);
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

char* SlabAlloc::dump() {
    char* buf = new char[4096];
    int pos = 0;
    for (int i = 0; i < SLAB_FREE_NUM; ++i) {
        SlabInfo* slab = _free[i];
        if (slab) {
            pos += sprintf(buf + pos, "[%02d][%p]prev: %p, next: %p, allocated: %03d, free: %p\n",
                           i, slab, slab->prev, slab->next, slab->allocated, slab->free);
            SlabInfo* next = slab->next;
            while (next != slab) {
                pos += sprintf(buf + pos, "[%02d][%p]prev: %p, next: %p, allocated: %03d, free: %p\n",
                               i, next, next->prev, next->next, next->allocated, next->free);
                next = next->next;
            }
        }
    }
    return buf;
}

} /* namespace memory */
