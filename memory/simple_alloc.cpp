#include "memory/simple_alloc.h"

namespace memory {

pthread_key_t Allocater::_localKey;
static Allocater _galloc(4096 << 8, 512);

void *allocate(size_t size) {
    Allocater *localAlloc = (Allocater*)pthread_getspecific(Allocater::getLocalKey());
    if (localAlloc) {
        void *addr = localAlloc->alloc(size);
        if (addr) {
            return addr;
        }
    }

    return _galloc.mutexAlloc(size);
}

void deallocate(const void *addr, size_t size) {
    Allocater *localAlloc = (Allocater*)pthread_getspecific(Allocater::getLocalKey());
    if (localAlloc && localAlloc->contains(addr)) {
        return localAlloc->free(addr, size);
    }

    return _galloc.mutexFree(addr, size);
}

} /* namespace memory */
