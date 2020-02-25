#include "memory/simple_alloc.h"

namespace memory {

pthread_key_t *Allocater::_localKey = NULL;
pthread_key_t *Allocater::_globalKey = NULL;
static Allocater _galloc(4096, 256);

void *allocate(size_t size) {
    if (!Allocater::getLocalKey() && !Allocater::getGlobalKey()) {
        return _galloc.alloc(size);
    }
    Allocater *localAlloc = (Allocater*)pthread_getspecific(*Allocater::getLocalKey());
    if (localAlloc) {
        return localAlloc->alloc(size);
    }
    
    Allocater *globalAlloc = (Allocater*)pthread_getspecific(*Allocater::getGlobalKey());
    return globalAlloc->alloc(size);
}

void deallocate(const void *addr, size_t size) {
    if (!Allocater::getLocalKey() && !Allocater::getGlobalKey()) {
        _galloc.free(addr, size);
        return;
    }
    Allocater *localAlloc = (Allocater*)pthread_getspecific(*Allocater::getLocalKey());
    if (localAlloc) {
        return localAlloc->free(addr, size);
    }
    
    Allocater *globalAlloc = (Allocater*)pthread_getspecific(*Allocater::getGlobalKey());
    return globalAlloc->free(addr, size);
}

} /* namespace memory */
