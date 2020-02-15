#include "config.h"
#include "memory/buddy.h"
#include <sys/mman.h>
#include <unistd.h>

#define INVALID     -1

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

Buddy::~Buddy() {
    munmap(_buffer, _size);
}

void Buddy::init(int blocks, int blockSize) {
    if (_buffer) {
        return;
    }
    
    _blockShiftBit = pow(blockSize);
    _blockPow = pow(blocks);
    blocks = 1 << _blockPow;
    int nodes = (blocks << 1) - 1;
    _size = blocks * (1 << _blockShiftBit) + nodes;
    _buffer = (char*)mmap(NULL, _size, MMAP_PROT, MMAP_FLAGS, -1, 0);
    _tree = _buffer + _size - nodes;
    _tree[0] = _blockPow;
    for (int i = 1; i < nodes; ++i) {
        _tree[i] = _tree[parent(i)] - 1;
    }
}

void* Buddy::alloc(int size) {
    int mask = (1 << _blockShiftBit) - 1;
    char p1 = pow((size + mask & ~mask) >> _blockShiftBit);
    if (_blockPow < p1) {
        return NULL;
    }
    
    int idx = 0;
    char p2 = _blockPow;
    while (p1 != p2--) {
        idx = leftChild(idx);
        if (_tree[idx] < p1) {
            ++idx;
        }
	}
    _tree[idx] = INVALID;
    
    int offset = (idx + 1) * (1 << p1) - (1 << _blockPow);
    while (idx) {
        idx = parent(idx);
        int child = leftChild(idx);
        _tree[idx] = MAX(_tree[child], _tree[child + 1]);
    }

    return _buffer + offset * (1 << _blockShiftBit);
}

void Buddy::free(void *addr) {
    int offset = int(((char*)addr - _buffer) >> _blockShiftBit);
    int idx = offset + (1 << _blockPow) - 1;
    char n = 0;
    for (; _tree[idx] != INVALID; idx = parent(idx)) {
        ++n;
        if (idx == 0) return;
    }
    _tree[idx] = n;

    while(idx) {
        idx = parent(idx);
        ++n;
        char left = _tree[leftChild(idx)];
        char right = _tree[leftChild(idx) + 1];
        if (left == n - 1 && right == n - 1)
            _tree[idx] = n;
        else
            _tree[idx] = MAX(left, right);
    }
}

char* Buddy::dump() {
    int blocks = 1 << _blockPow;
    int nodes = (blocks << 1) - 1;
    int n = _blockPow;
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
