#ifndef __MEMORY_BUDDY_H__
#define __MEMORY_BUDDY_H__

namespace memory {

class Buddy {
public:
    Buddy(): _buffer(0), _tree(0), _size(0), _blockShiftBit(0), _blockPow(0) {}
    explicit Buddy(int blocks, int blockSize): _buffer(0), _tree(0), _size(0), _blockShiftBit(0), _blockPow(0) {
        init(blocks, blockSize);
    }
    ~Buddy();
    void init(int blocks, int blockSize);
    void* alloc(int size);
    void free(void* addr);
    char* dump();
    char* buffer() const { return _buffer; }

private:
    char* _buffer;
    char* _tree;
    int _size;
    char _blockShiftBit;
    char _blockPow;
};

} /* namespace memory */

#endif /* ifndef __MEMORY_BUDDY_H__ */
