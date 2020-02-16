#ifndef __MEMORY_BUDDY_H__
#define __MEMORY_BUDDY_H__

namespace memory {

class BuddyMalloc {
public:
    BuddyMalloc(): _buffer(0), _tree(0), _size(0), _blockShiftBit(0), _blockPow(0) {}
    explicit BuddyMalloc(int blocks, int blockSize): _buffer(0), _tree(0), _size(0), _blockShiftBit(0), _blockPow(0) {
        init(blocks, blockSize);
    }
    ~BuddyMalloc();
    void init(int blocks, int blockSize);
    void *alloc(int size);
    void free(const void *addr);
    char *dump();
    char *buffer() const { return _buffer; }

private:
    char *_buffer;
    char *_tree;
    int _size;
    char _blockShiftBit;
    char _blockPow;
};

} /* namespace memory */

#endif /* ifndef __MEMORY_BUDDY_H__ */
