#include "memory/buddy.h"

#define LCHILD(i)		(((i) << 1) + 1)
#define PARENT(i)		((i - 1) >> 1)
#define PAGES(n)        (1 << (n - 1))

BEGIN_NS(memory)

Buddy::~Buddy() {
    delete[] _buffer;
    delete[] _tree;
    _buffer = NULL;
    _tree = NULL;
}

void Buddy::init(size_t pages) {
    if (_buffer) {
        return;
    }
    _depth = LOG2PLUS1(pages);
    pages = 1 << _depth;
    size_t nodes = (pages << 1) - 1;
    _pages = _buffer = new char[(pages + 1) * PAGE_SIZE];
    intptr_t addr = ALIGN_PAGE((intptr_t)_pages);
    if (addr != (intptr_t)_pages) {
        _pages = (char*)addr;
    }
    _tree = new uint8_t[nodes];
    _tree[0] = _depth;
    for (size_t i = 1; i < nodes; ++i) {
        _tree[i] = _tree[PARENT(i)] - 1;
    }
}

void *Buddy::alloc(size_t pages) {
    uint8_t n1 = LOG2PLUS1(pages);
    size_t idx = 0;
	if (pages == 0 || _tree[idx] < n1) {
	    return NULL;
	}
    uint8_t n2 = _depth + 1;
    while (n1 != --n2) {
        idx = LCHILD(idx);
        if (_tree[idx] < n1) {
            ++idx;
        }
	}
    _tree[idx] = 0;
	size_t offset = (idx + 1) * PAGES(n2) - PAGES(_depth);
    while (idx) {
        idx = PARENT(idx);
        size_t child = LCHILD(idx);
        _tree[idx] = MAX(_tree[child], _tree[child + 1]);
    }

    return _pages + offset * PAGE_SIZE;
}

void Buddy::free(void *addr) {
    char *paddr = (char*)addr;
    size_t pages = PAGES(_depth);
    char *end = _pages + pages * PAGE_SIZE;
    if (paddr < _pages || paddr > end) {
        return;
    }

    size_t offset = (paddr - _pages) / PAGE_SIZE;
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

char *Buddy::dump() {
    size_t pages = PAGES(_depth);
    size_t nodes = (pages << 1) - 1;
    size_t n = LOG2PLUS1(pages) + 1;
    char *buf = new char[pages + 1];
    
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

END_NS
