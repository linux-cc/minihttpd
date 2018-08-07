#ifndef __UTILS_CYCLE_QUEUE_H__
#define __UTILS_CYCLE_QUEUE_H__

#include "config.h"

BEGIN_NS(memory)
class FixedMalloc;
END_NS

BEGIN_NS(utils)

USING_CLASS(memory, FixedMalloc);

class CycleQueue {
public:
    class ItemList {
    public:
        int length() const { return _length; }
        const void *data() const { return _data; }
        const ItemList *next() const { return _next; }
    private:
        int _length;
        ItemList *_next;
        char _data[1];
        friend class CycleQueue;
    };

    CycleQueue(FixedMalloc &alloc, int capacity = 1024):
        _alloc(alloc), _capacity(capacity), _items(NULL), _size(0), _head(0), _tail(0) {
        _items = new Node[capacity];
    }
    ~CycleQueue() {
        delete []_items;
    }
    bool enQueue(const void *buf, int size);
    ItemList *deQueue();
    void freeItemList(ItemList *list);

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
    void enQueueSingle(const void *buf, int size);
    void enQueueMultiple(const void *buf, int size);

    struct Node {
        ItemList *head;
    };
    FixedMalloc &_alloc;
    int _capacity;
    Node *_items;
    int _size;
    volatile int _head;
    volatile int _tail;
};

END_NS
#endif /* ifndef __UTILS_CYCLE_QUEUE_H__ */
