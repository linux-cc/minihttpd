#include "memory/fixed_malloc.h"
#include "memory/buddy.h"

namespace memory {

FixedMalloc::FixedMalloc(Buddy& buddy):
_buddy(buddy),
_elem(0),
_size(0),
_free(0),
_used(0),
_buffer(NULL) {
}

bool FixedMalloc::init(int size, int elem) {
    _buffer = (char*)_buddy.alloc(size * elem);
    if(!_buffer) {
        return false;
    }
    _elem = elem;
    _size = size;

    char* p = _buffer;
    for(int i = 1; i < _size ; ++i) {
        *(uint16_t*)p = i;
        p += _elem;
    }

    return true;
}

void* FixedMalloc::alloc(int size) {
    if(size > _elem || _used >= _size) {
        return NULL;
    }

    char* addr = _buffer + _free * _elem;
    _free = *(uint16_t*)addr;
    ++_used;

    return addr;
}

void FixedMalloc::free(void* addr) {
    char* p = (char*)addr;
    if(p >= _buffer && p <= _buffer + _size * _elem) {
        *(uint16_t*)p = _free;
        _free = (p - _buffer) / _elem;
        --_used;
    }
}

void FixedMalloc::destroy() {
    _buddy.free(_buffer);
    _buffer = NULL;
    _elem = 0;
    _size = 0;
    _free = 0;
    _used = 0;
}

} /* namespace memory */
