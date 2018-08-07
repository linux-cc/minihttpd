#include "memory/fixed_malloc.h"
#include "memory/buddy.h"

BEGIN_NS(memory)

FixedMalloc::FixedMalloc(Buddy &buddy):
_buddy(buddy),
_elem(0),
_size(0),
_free(0),
_used(0),
_buffer(NULL) {
}

void *FixedMalloc::alloc(size_t size) {
    if(size > _elem || _used >= _size) {
        return NULL;
    }

    char *addr = _buffer + _free * _elem;
    _free = *(uint16_t*)addr;
    ++_used;

    return addr;
}

void FixedMalloc::free(void *addr) {
    char *p = (char *)addr;
    if(p >= _buffer && p <= _buffer + _size * _elem) {
        *(uint16_t*)p = _free;
        _free = (p - _buffer) / _elem;
        --_used;
    }
}

bool FixedMalloc::init(size_t size, size_t elem) {
    int total = size * elem;
    int pages = total / PAGE_SIZE;
    if (total % PAGE_SIZE) {
        ++pages;
    }
    _buffer = (char*)_buddy.alloc(pages);
    if(!_buffer) {
        return false;
    }
    _elem = elem;
    _size = pages * PAGE_SIZE / _elem;

    char *p = _buffer;
    for(size_t i = 1; i < _size ; ++i) {
        *(uint16_t*)p = i;
        p += _elem;
    }

    return true;
}

void FixedMalloc::destroy() {
    _buddy.free(_buffer);
    _buffer = NULL;
    _elem = 0;
    _size = 0;
    _free = 0;
    _used = 0;
}

END_NS
