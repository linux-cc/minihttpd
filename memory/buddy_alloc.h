#ifndef __MEMORY_BUDDY_ALLOC_H__
#define __MEMORY_BUDDY_ALLOC_H__

#include <stddef.h>

namespace memory {

class BuddyAlloc {
public:
    BuddyAlloc(int blocks, int blockSize, int pageSize = 4096);
    ~BuddyAlloc();
    
    void init(int blocks, int blockSize, int pageSize);
    void *alloc(size_t size);
    void free(const void *addr);
    bool contains(const void *addr);
    size_t getPageSize() const { return _pageSize; }
    char *buffer() const { return _buffer; }
    char *dump();
    
private:
    char *_maddr;
    char *_buffer;
    char *_tree;
    size_t _size;
    size_t _pageSize;
    char _blockShiftBit;
    char _blocksPow;
};

} /* namespace memory */

#endif /* ifndef __MEMORY_BUDDYALLOC_H__ */
