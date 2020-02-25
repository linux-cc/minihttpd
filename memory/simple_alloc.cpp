#include "memory/simple_alloc.h"

namespace memory {

pthread_key_t *Allocater::_localKey = NULL;
pthread_key_t *Allocater::_globalKey = NULL;

void *allocate(size_t size) {
    Allocater *localAlloc = (Allocater*)pthread_getspecific(Allocater::getLocalKey());
    if (localAlloc) {
        return localAlloc->alloc(size);
    }
    
    Allocater *globalAlloc = (Allocater*)pthread_getspecific(Allocater::getGlobalKey());
    return globalAlloc->alloc(size);
}

void deallocate(const void *addr, size_t size) {
    Allocater *localAlloc = (Allocater*)pthread_getspecific(Allocater::getLocalKey());
    if (localAlloc) {
        return localAlloc->free(addr, size);
    }
    
    Allocater *globalAlloc = (Allocater*)pthread_getspecific(Allocater::getGlobalKey());
    return globalAlloc->free(addr, size);
}

} /* namespace memory */
