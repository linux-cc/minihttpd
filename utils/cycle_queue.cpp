#include "utils/cycle_queue.h"
#include "memory/fixed_malloc.h"

BEGIN_NS(utils)

bool CycleQueue::enQueue(const void *buf, int size) {
    if (!full() && _alloc.canAlloc(size)) {
        if (size <= _alloc.elem()) {
            enQueueSingle(buf, size);
        } else {
            enQueueMultiple(buf, size);
        }
        return true;
    }

    return false;
}

CycleQueue::ItemList *CycleQueue::deQueue() {
    ItemList *head = NULL;
    if (!empty()) {
        head = _items[_head].head;
        _head + 1 == _capacity ? (_head = 0) : ++_head;
    }
    return head;
}

void CycleQueue::freeItemList(ItemList *list) {
    while (list) {
        ItemList *next = list->_next;
        _alloc.free(list);
        list = next;
    }
}

void CycleQueue::enQueueSingle(const void *buf, int size) {
    ItemList *curr = (ItemList*)_alloc.alloc();
    memcpy(curr->_data, buf, size);
    curr->_length = size;
    curr->_next = NULL;
    _items[_tail].head = curr;
    _tail + 1 == _capacity ? (_tail = 0) : ++_tail;
}

void CycleQueue::enQueueMultiple(const void *buf, int size) {
    char *pb = (char*)buf;
    ItemList *head = NULL, *prev = NULL;
    int len = _alloc.elem();
    while (size) {
        ItemList *curr = (ItemList*)_alloc.alloc();
        if (!head) {
            head = curr;
        }
        if (prev) {
            prev->_next = curr;
        }
        int length = size > len ? len : size;
        memcpy(curr->_data, pb, length);
        curr->_length = length;
        if (size <= len) {
            curr->_next = NULL;
            break;
        }
        pb += length;
        size -= length;
        prev = curr;
    }
    _items[_tail].head = head;
    _tail + 1 == _capacity ? (_tail = 0) : ++_tail;
}

END_NS
