#include "config.h"
#include "memory/buddy.h"
#include <sys/mman.h>
#include <unistd.h>

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

static inline bool isPow2(int n) {
    return !(n & (n - 1));
}

static inline int depth(int n) {
    return sizeof(int) * __CHAR_BIT__ - __builtin_clz(n) + (isPow2(n) ? 0 : 1);
}

static inline int leftChild(int i) {
    return (i << 1) + 1;
}

static inline int parent(int i) {
    return (i - 1) >> 1;
}

static inline int pagesNum(int depth) {
    return 1 << (depth - 1);
}

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
    _depth = depth(pages);
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
        _tree[i] = _tree[parent(i)] - 1;
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
    uint8_t n1 = depth(pages);
    size_t idx = 0;
	if (pages == 0 || _tree[idx] < n1) {
	    return NULL;
	}
    uint8_t n2 = _depth;
    while (n1 != n2--) {
        idx = leftChild(idx);
        if (_tree[idx] < n1) {
            ++idx;
        }
	}
    _tree[idx] = 0;
	size_t offset = (idx + 1) * pagesNum(n1) - pagesNum(_depth);
    while (idx) {
        idx = parent(idx);
        size_t child = leftChild(idx);
        _tree[idx] = MAX(_tree[child], _tree[child + 1]);
    }

    return _buffer + offset * _pageSize;
}

void Buddy::free(void* addr) {
    char* paddr = (char*)addr;
    size_t pages = pagesNum(_depth);
    char* end = _buffer + pages * _pageSize;
    if (paddr < _buffer || paddr > end) {
        return;
    }

    size_t offset = (paddr - _buffer) / _pageSize;
    size_t idx = offset + pages - 1;
    uint8_t n = 1;
    for (; _tree[idx]; idx = parent(idx)) {
        ++n;
        if (idx == 0)
            return;
    }
    _tree[idx] = n;

    while(idx) {
        idx = parent(idx);
        ++n;
        uint8_t left = _tree[leftChild(idx)];
        uint8_t right = _tree[leftChild(idx) + 1];
        if (left == n - 1 && right == n - 1)
            _tree[idx] = n;
        else
            _tree[idx] = MAX(left, right);
    }
}

char* Buddy::dump() {
    size_t pages = pagesNum(_depth);
    size_t nodes = (pages << 1) - 1;
    size_t n = _depth + 1;
    char* buf = new char[pages + 1];
    for (size_t i = 0; i < pages; ++i)
        buf[i] = '_';
    for (size_t i = 0; i < nodes; ++i) {
        if (isPow2(i+1))
            --n;
        if (_tree[i]) 
            continue;
        if (i >= pages - 1) {
            buf[i - pages + 1] = '*';
        } else if (_tree[leftChild(i)] && _tree[leftChild(i) + 1]) {
            size_t nsize = pagesNum(n);
            size_t offset = (i + 1) * nsize - pages;
            for (size_t j = offset; j < offset + nsize; ++j)
                buf[j] = '*';
        }
    }
    buf[pages] = '\0';
    return buf;
}

} /* namespace memory */
