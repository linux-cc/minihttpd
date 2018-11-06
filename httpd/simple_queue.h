#ifndef __HTTPD_SIMPLE_QUEUE_H__
#define __HTTPD_SIMPLE_QUEUE_H__

#include <stdlib.h>

namespace httpd {

template<typename Type>
class SimpleQueue {
public:
    SimpleQueue(): _items(NULL), _capacity(0), _size(0), _head(0), _tail(0) {}
    ~SimpleQueue() {
        if (_items) {
            delete []_items;
        }
    }
    void init(Type *items, int nitem) {
        _capacity = nitem;
        _items = new Type*[_capacity];
        for (int i = 0; items && i < _capacity; ++i) {
            pushBack(items + i);
        }
    }
    void pushBack(Type *item) {
        if (!full()) {
            _items[_tail++] = item;
            if (_tail == _capacity) {
                _tail = 0;
            }
            ++_size;
        }
    }
    Type *popFront() {
        Type *t = NULL;
        if (!empty()) {
            t = _items[_head++];
            if (_head == _capacity) {
                _head = 0;
            }
            --_size;
        }
        return t;
    }
    Type *at(int index) {
        return _items[index];
    }
    bool empty() const {
        return _size == 0;
    }
    bool full() const {
        return _size && _head == _tail;
    }
    int size() const {
        return _size;
    }
    int capacity() const {
        return _capacity;
    }
    
private:
    Type **_items;
    int _capacity;
    int _size;
    int _head;
    int _tail;
};

} /* namespace httpd */
#endif /* ifndef __HTTPD_SIMPLE_QUEUE_H__ */
