#include "memory/allocater.h"
#include "memory/buddy_malloc.h"
#include "memory/slab_malloc.h"

#define BLOCK_SIZE      256

namespace memory {

static BuddyMalloc _buddy(4096 << 1, BLOCK_SIZE << 1);
static SlabMalloc _slab(_buddy);

void *allocate(int size) {
    return size > BLOCK_SIZE ? _buddy.alloc(size) : _slab.alloc(size);
}

void deallocate(const void *addr, int size) {
    size > BLOCK_SIZE ? _buddy.free(addr) : _slab.free(addr);
}

} /* namespace memory */
