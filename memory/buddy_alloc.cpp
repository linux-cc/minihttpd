#include "util/config.h"
#include "memory/buddy_alloc.h"
#include <sys/mman.h>
#include <unistd.h>

#define INVALID             -1
#define BLOCK_MASK          ((1 << _blockShiftBit) - 1)
#define BLOCK_ALIGN(n)      (((n) + BLOCK_MASK) & ~BLOCK_MASK)
#define BLOCK_NUM(n)        (BLOCK_ALIGN(n) >> _blockShiftBit)
#define PAGE_ALIGN(n)       (((n) + _pageSize - 1) & ~(_pageSize - 1))

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

static inline bool isPow(int n) {
    return !(n & (n - 1));
}

static inline int pow(int n) {
    return sizeof(int) * __CHAR_BIT__ - __builtin_clz(n) - (isPow(n) ? 1 : 0);
}

static inline int leftChild(int i) {
    return (i << 1) + 1;
}

static inline int parent(int i) {
    return (i - 1) >> 1;
}

BuddyAlloc::~BuddyAlloc() {
    munmap(_maddr, _size);
}

void BuddyAlloc::init(int blocks, int blockSize, int pageSize) {
    if (_maddr) {
        return;
    }
    
    _pageSize = (int)sysconf(_SC_PAGESIZE);
    if (pageSize > _pageSize) {
        _pageSize = PAGE_ALIGN(pageSize);
    }
    _blockShiftBit = pow(blockSize);
    _blocksPow = pow(blocks);
    blocks = 1 << _blocksPow;
    blockSize = 1 << _blockShiftBit;
    int nodes = (blocks << 1) - 1;
    _size = PAGE_ALIGN(blocks * blockSize + nodes);
    _maddr = (char*)mmap(NULL, _size, MMAP_PROT, MMAP_FLAGS, -1, 0);
    _buffer = (char*)PAGE_ALIGN((intptr_t)_maddr);
    _tree = _buffer - _maddr > nodes ? _maddr : _maddr + _size - nodes;
    _tree[0] = _blocksPow;
    for (int i = 1; i < nodes; ++i) {
        _tree[i] = _tree[parent(i)] - 1;
    }
    _LOG_("blockShiftBit: %d(%d), blockPow: %d(%d), buffer: %p, pageSize: %d", _blockShiftBit, blockSize, _blocksPow, blocks, _buffer, _pageSize);
}

void* BuddyAlloc::alloc(int size) {
    int idx = 0;
    char p1 = pow(BLOCK_NUM(size));
    if (_tree[idx] < p1) {
        return NULL;
    }
    
    char p2 = _blocksPow;
    while (p1 != p2--) {
        idx = leftChild(idx);
        if (_tree[idx] < p1) {
            ++idx;
        }
	}
    _tree[idx] = INVALID;
    
    int offset = (idx + 1) * (1 << p1) - (1 << _blocksPow);
    _LOG_("size: %d, pow(size): %d(%d blocks), idx: %d, offset: %d, alloc: %p",
          size, p1, 1 << p1, idx, offset, _buffer + offset * (1 << _blockShiftBit));
    while (idx) {
        idx = parent(idx);
        int child = leftChild(idx);
        _tree[idx] = MAX(_tree[child], _tree[child + 1]);
    }

    return _buffer + offset * (1 << _blockShiftBit);
}

void BuddyAlloc::free(const void *addr) {
    int offset = int(((char*)addr - _buffer) >> _blockShiftBit);
    int idx = offset + (1 << _blocksPow) - 1;
    char n = 0;
    for (; _tree[idx] != INVALID; idx = parent(idx)) {
        ++n;
        if (idx == 0) return;
    }
    _tree[idx] = n;
    _LOG_("addr: %p, idx: %d, offset: %d", addr, idx, offset);
    while (idx) {
        idx = parent(idx);
        ++n;
        char left = _tree[leftChild(idx)];
        char right = _tree[leftChild(idx) + 1];
        _tree[idx] = (left == n - 1 && right == n - 1) ? n : MAX(left, right);
    }
}

char* BuddyAlloc::dump() {
    int blocks = 1 << _blocksPow;
    int nodes = (blocks << 1) - 1;
    int n = _blocksPow;
    char *buf = new char[blocks + 1];
    for (int i = 0; i < blocks; ++i)
        buf[i] = '_';
    for (int i = 0; i < nodes; ++i) {
        if (i && isPow(i+1))
            --n;
        if (_tree[i] != INVALID)
            continue;

        if (i >= blocks - 1) {
            buf[i - blocks + 1] = '*';
        } else if (_tree[leftChild(i)] != INVALID && _tree[leftChild(i) + 1] != INVALID) {
            int nsize = 1 << n;
            int offset = (i + 1) * nsize - blocks;
            for (int j = offset; j < offset + nsize; ++j)
                buf[j] = '*';
        }
    }
    buf[blocks] = '\0';
    return buf;
}

} /* namespace memory */
