#include "memory/allocater.h"
#include "memory/buddy_alloc.h"
#include "memory/slab_alloc.h"

namespace memory {

static BuddyAlloc _buddy(4096 << 1, SlabAlloc::maxSlabSize() << 1);
static SlabAlloc _slab(_buddy);

void *allocate(int size) {
    return _slab.alloc(size);
}

void deallocate(const void *addr, int size) {
    _slab.free(addr, size);
}

} /* namespace memory */
