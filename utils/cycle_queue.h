#ifndef __UTILS_CYCLE_QUEUE_H__
#define __UTILS_CYCLE_QUEUE_H__

#include "config.h"

BEGIN_NS(utils)

template<typename Type>
class CycleQueue {
public:
    CycleQueue(): _items(NULL), _capacity(0), _size(0), _head(0), _tail(0) {}
    ~CycleQueue() {
        if (_items) {
            delete []_items;
        }
    }
    void init(Type *items, int nitem) {
        _capacity = nitem;
        _items = new Item[_capacity];
        for (int i = 0; items && i < _capacity; ++i) {
            pushBack(items + i);
        }
    }
    void pushBack(Type *item) {
        if (!full()) {
            _items[_tail++].item = item;
            if (_tail == _capacity) {
                _tail = 0;
            }
            ++_size;
        }
    }
    Type *popFront() {
        Type *t = NULL;
        if (!empty()) {
            t = _items[_head++].item;
            if (_head == _capacity) {
                _head = 0;
            }
            --_size;
        }
        return t;
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
    
private:
    struct Item {
        Type *item;
    };
    Item *_items;
    int _capacity;
    int _size;
    int _head;
    int _tail;
};

END_NS
#endif /* ifndef __UTILS_CYCLE_QUEUE_H__ */
