#include "config.h"
#include "memory/buddy.h"
#include <sys/mman.h>
#include <unistd.h>

#define SIZE_BITS           (sizeof(unsigned) * __CHAR_BIT__)
#define IS_POW2(n)          (!((n) & ((n) - 1)))
#define LOG2(n)             (SIZE_BITS - __builtin_clz(n) - (IS_POW2(n) ? 1 : 0))
#define LOG2PLUS1(n)        (LOG2(n) + 1)
#define LCHILD(i)		    (((i) << 1) + 1)
#define PARENT(i)		    ((i - 1) >> 1)
#define PAGES(n)            (1 << (n - 1))

namespace memory {

/*
static uint32_t builtin_clz(uint32_t x) {
    uint32_t n = SIZE_BITS, s = n >> 1, y;
    while (s) {
        y = x >> s;
        if (y) {
            n -= s;
            x = y;
        }
        s >>= 1;
    }
    return n - 1;
}
*/

Buddy::~Buddy() {
    munmap(_mmap, _msize);
}

void Buddy::init(size_t pages) {
    if (_mmap) {
        return;
    }
    _pageSize = sysconf(_SC_PAGESIZE);
    if (_pageSize == (size_t)-1) {
        return;
    }
    _depth = LOG2PLUS1(pages);
    pages = 1 << _depth;
    size_t nodes = (pages << 1) - 1;
    _msize = (pages + 1) * _pageSize + nodes;
    _buffer = _mmap = (char*)mmap(NULL, _msize, MMAP_PROT, MMAP_FLAGS, -1, 0);
    intptr_t addr = ((intptr_t)_buffer + pageMask()) & ~pageMask();
    if (addr != (intptr_t)_buffer) {
        _buffer = (char*)addr;
    }
    _tree = (uint8_t*)_buffer + _msize - nodes;
    _tree[0] = _depth;
    for (size_t i = 1; i < nodes; ++i) {
        _tree[i] = _tree[PARENT(i)] - 1;
    }
}

void* Buddy::alloc(size_t size) {
    size_t pages = size / _pageSize;
    if (size & pageMask()) {
        ++pages;
    }

    return allocPages(pages);
}

void* Buddy::allocPages(size_t pages) {
    uint8_t n1 = LOG2PLUS1(pages);
    size_t idx = 0;
	if (pages == 0 || _tree[idx] < n1) {
	    return NULL;
	}
    uint8_t n2 = _depth;
    while (n1 != n2--) {
        idx = LCHILD(idx);
        if (_tree[idx] < n1) {
            ++idx;
        }
	}
    _tree[idx] = 0;
	size_t offset = (idx + 1) * PAGES(n1) - PAGES(_depth);
    while (idx) {
        idx = PARENT(idx);
        size_t child = LCHILD(idx);
        _tree[idx] = MAX(_tree[child], _tree[child + 1]);
    }

    return _buffer + offset * _pageSize;
}

void Buddy::free(void* addr) {
    char* paddr = (char*)addr;
    size_t pages = PAGES(_depth);
    char* end = _buffer + pages * _pageSize;
    if (paddr < _buffer || paddr > end) {
        return;
    }

    size_t offset = (paddr - _buffer) / _pageSize;
    size_t idx = offset + pages - 1;
    uint8_t n = 1;
    for (; _tree[idx]; idx = PARENT(idx)) {
        ++n;
        if (idx == 0)
            return;
    }
    _tree[idx] = n;

    while(idx) {
        idx = PARENT(idx);
        ++n;
        uint8_t left = _tree[LCHILD(idx)];
        uint8_t right = _tree[LCHILD(idx) + 1];
        if (left == n - 1 && right == n - 1)
            _tree[idx] = n;
        else
            _tree[idx] = MAX(left, right);
    }
}

char* Buddy::dump() {
    size_t pages = PAGES(_depth);
    size_t nodes = (pages << 1) - 1;
    size_t n = LOG2PLUS1(pages) + 1;
    char* buf = new char[pages + 1];
    for (size_t i = 0; i < pages; ++i)
        buf[i] = '_';
    for (size_t i = 0; i < nodes; ++i) {
        if (IS_POW2(i+1))
            --n;
        if (_tree[i]) 
            continue;
        if (i >= pages - 1) {
            buf[i - pages + 1] = '*';
        } else if (_tree[LCHILD(i)] && _tree[LCHILD(i) + 1]) {
            size_t nsize = PAGES(n);
            size_t offset = (i + 1) * nsize - pages;
            for (size_t j = offset; j < offset + nsize; ++j)
                buf[j] = '*';
        }
    }
    buf[pages] = '\0';
    return buf;
}

} /* namespace memory */
