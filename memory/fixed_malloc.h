#ifndef __MEMORY_FIXED_MALLOC_H__
#define __MEMORY_FIXED_MALLOC_H__

namespace memory {

class Buddy;

class FixedMalloc {
public:
    FixedMalloc(Buddy &buddy);

    bool init(int size, int elem);
    void* alloc(int size);
    void* alloc() { return alloc(_elem); }
    void free(void* addr);
    void destroy();
    int elem() const { return _elem;  }
    bool canAlloc(int size) { return (_size - _used) * _elem >= size; }

private:
    Buddy& _buddy;
    int _elem;
    int _size;
    int _free;
    int _used;
    char* _buffer;
};

} /* namespace memory */
#endif /* ifndef __MEMORY_FIXED_MALLOC_H__ */
