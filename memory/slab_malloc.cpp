#include "slab_malloc.h"
#include <cstdio>

#define SLAB_MAX_SIZE       (MAX_FREE_LIST << ALIGN8_SHIFT)
#define SLAB_FREE_IDX(n)    (((n) >> ALIGN8_SHIFT) - 1)

namespace memory {

SlabMalloc::SlabMalloc(Buddy *_buddy):
buddy(_buddy) {
    for (int i = 0; i < MAX_FREE_LIST; ++i) {
        freeList[i] = NULL;
    }
}

void *SlabMalloc::alloc(size_t size) {
    size = ALIGN8(size);
    if (size > SLAB_MAX_SIZE) {
        return NULL;
    }
    size_t idx = SLAB_FREE_IDX(size);
    if (!freeList[idx] || !freeList[idx]->free) {
        void *page = buddy->alloc(1);
        if (!page) {
            return NULL;
        }
        initPage(idx, page);
    }
    Info *slab = freeList[idx];
    Chunk *chunk = slab->free;
    slab->free = chunk->next;
    ++slab->allocated;
    if (!slab->free) {
        freeList[idx] = slab->next;
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
    Info *header = freeList[idx];
    if (!header) {
        slab->next = slab;
        slab->prev = slab;
    } else {
        slab->prev = header->prev;
        slab->prev->next = slab;
        slab->next = header;
        slab->next->prev = slab;
    }
    freeList[idx] = slab;
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
        if (freeList[idx] == slab) {
            freeList[idx] = next == slab ? NULL : next;
        }
        if (wasEmpty) {
            linkSlab(idx, slab);
        } else {
            buddy->free(slab);
        }
    }
}

char *SlabMalloc::dump() {
    char *buf = new char[4096];
    int pos = 0;
    for (int i = 0; i < MAX_FREE_LIST; ++i) {
        Info *slab = freeList[i];
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

}

#ifdef _SLAB_MAIN_

#include <vector>

int main(int argc, char *argv[]) {
    using namespace memory;
    using std::vector;
    vector<void*> v;
    Buddy buddy(8);
    SlabMalloc slab(&buddy);
    for (;;) {
        int cmd, size;
        printf("enter command:");
        scanf("%d %d", &cmd, &size);
        if (cmd == 1) {
            void *p = slab.alloc(size);
            if (p) {
                v.push_back(p);
                printf("alloc: %p, %d\n", p, size);
            }
        }
        if (cmd == 0) {
            if (v.empty())
                continue;
            vector<void*>::iterator it = v.begin();
            void *p = *(it + size);
            slab.free(p);
            v.erase(it + size);
            printf("free : %p\n", p);
        }
        printf("alloc list: ");
        for (int i = 0; i < v.size(); ++i)
            printf("%p, ", v[i]);
        printf("\nbuddy dump: %s\n", buddy.dump());
        printf("slab dump:\n%s\n", slab.dump());
    }
    return 0;
}

#endif
