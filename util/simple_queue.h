#ifndef __UTIL_SIMPLE_QUEUE_H__
#define __UTIL_SIMPLE_QUEUE_H__

#include "memory/simple_alloc.h"
#include "thread/thread.h"

namespace util {

template <typename T>
class SimpleQueue {
public:
    SimpleQueue(int capacity): _capacity(capacity), _size(0), _head(0), _tail(0) { _queue = memory::SimpleAlloc<T[]>::New(_capacity); }
    ~SimpleQueue() { memory::SimpleAlloc<T[]>::Delete(_queue, _capacity); }
    
    bool empty() const { return _size == 0; }
    bool full() const { return _size == _capacity; }
    int size() const { return _size; }
    int capacity() const { return _capacity; }
    T &at(int index) { return _queue[index]; }
    
    bool enqueue(const T &data) {
        if (full()) return false;
        
        _queue[_tail++] = data;
        if (_tail == _capacity) {
            _tail = 0;
        }
        ++_size;
        
        return true;
    }
    
    bool dequeue(T &data) {
        if (empty()) return false;
        
        data = _queue[_head++];
        if (_head == _capacity) {
            _head = 0;
        }
        --_size;
        
        return true;
    }
    
private:
    T *_queue;
    int _capacity;
    int _size;
    int _head;
    int _tail;
};

template <typename T>
class BlockQueue {
public:
    BlockQueue(int capacity): _queue(capacity) { }
    
    bool enqueue(const T &data) {
        thread::AutoMutex mutex(_mutex);
        return _queue.enqueue(data);
    }
    
    bool dequeue(T &data) {
        thread::AutoMutex mutex(_mutex);

        return _queue.dequeue(data);
    }
    
private:
    SimpleQueue<T> _queue;
    thread::Mutex _mutex;
};

}

#endif /* __UTIL_SIMPLE_QUEUE_H__ */
