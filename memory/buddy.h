#ifndef __MEMORY_BUDDY_H__
#define __MEMORY_BUDDY_H__

#include <stdlib.h>

namespace memory {

class Buddy {
public:
    Buddy(): _mmap(NULL), _tree(NULL), _msize(0), _pageSize(0), _depth(0) {}
    explicit Buddy(size_t pages): _mmap(NULL), _tree(NULL), _msize(0), _pageSize(0), _depth(0) {
        init(pages);
    }
    ~Buddy();
    void init(size_t pages);
    void* alloc(size_t size);
    void* alloc() { return allocPages(1); }
    void* allocPages(size_t pages);
    void free(void* addr);
    size_t pageSize() const { return _pageSize; }
    size_t pageMask() const { return _pageSize - 1; }
    char* dump();
    char* buffer() const { return _mmap; }

private:
    char* _mmap;
    char* _buffer;
    uint8_t* _tree;
    size_t _msize;
    size_t _pageSize;
    uint8_t _depth;
};

} /* namespace memory */

#endif /* ifndef __MEMORY_BUDDY_H__ */
