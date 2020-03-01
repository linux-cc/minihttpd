#ifndef __UTIL_LOCK_FREE_LIST__
#define __UTIL_LOCK_FREE_LIST__

#include "memory/simple_alloc.h"

namespace util {

template <typename T>
class SimpleList {
public:
    SimpleList(): _size(0) {
        _head = memory::SimpleAlloc<Node>::New();
        _head->_prev = _head->_next = _head;
    }
    
    ~SimpleList() {
        clear();
        memory::SimpleAlloc<Node>::Delete(_head);
    }
    
    bool empty() const { return _head->_next == _head; }
    size_t size() const { return _size; }
    
    bool push(const T &data) {
        Node *n = memory::SimpleAlloc<Node>::New(data);
        if (!n) return false;
        
        Node *tail = _head->_prev;
        tail->_next = n;
        n->_prev = tail;
        _head->_prev = n;
        n->_next = _head;
        ++_size;
        
        return true;
    }
    
    bool pop(T &data) {
        if (empty()) return false;
        
        Node *n = _head->_next;
        Node *prev = n->_prev;
        Node *next = n->_next;
        prev->_next = next;
        next->_prev = prev;
        data = n->_data;
        memory::SimpleAlloc<Node>::Delete(n);
        --_size;
        
        return true;
    }
    
    bool find(const T &data) const {
        Node *n = _head->_next;
        while (n != _head) {
            if (n->_data == data) {
                return true;
            }
            n = n->_next;
        }
        
        return false;
    }
    
    void erase(const T &data) {
        Node *n = _head->_next;
        while (n != _head) {
            if (n->_data == data) {
                Node *prev = n->_prev;
                Node *next = n->_next;
                prev->_next = next;
                next->_prev = prev;
                memory::SimpleAlloc<Node>::Delete(n);
                --_size;
                return;
            }
            n = n->_next;
        }
    }
    
    void clear() {
        Node *n = _head->_next;
        while (n != _head) {
            Node *p = n;
            n = n->_next;
            memory::SimpleAlloc<Node>::Delete(p);
        }
    }
    
private:
    struct Node {
        Node() {}
        Node (const T &data): _data(data) {}
        T _data;
        Node *_prev;
        Node *_next;
    };
    
    Node *_head;
    size_t _size;
};

}

#endif /* __UTIL_LOCK_FREE_LIST__ */
