#ifndef __UTIL_SIMPLE_QUEUE_H__
#define __UTIL_SIMPLE_QUEUE_H__

#include "memory/simple_alloc.h"

namespace util {

using memory::SimpleAlloc;

template <typename T>
class Vector {
public:
    Vector(size_t size = 0) { resize(size); }
    ~Vector() { clear(); }
    
    bool empty() const { return _size == 0; }
    int size() const { return _size; }
    int capacity() const { return _capacity; }
    T &at(int index) const { return _array[index]; }
    
    bool pushBack(const T &data) {
        _array[_size] = data;
        
        return true;
    }
    
    bool dequeue(T &data) {
        
        return true;
    }
    
    void resize(size_t size) {
        if (size < _size) {
            
        }
        SimpleAlloc<T[]>::New(_capacity);
    }
    
    void clear() {
        
    }
    
private:
    T *_array;
    int _capacity;
    int _size;
};

}

#endif /* __UTIL_SIMPLE_QUEUE_H__ */
