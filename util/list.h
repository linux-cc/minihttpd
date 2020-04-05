#ifndef __UTIL_LOCK_FREE_LIST__
#define __UTIL_LOCK_FREE_LIST__

#include "memory/simple_alloc.h"

namespace util {

template <typename T>
class List {
public:
    List(): _size(0) {
        _head = memory::SimpleAlloc<Node>::New();
        _head->_prev = _head->_next = _head;
    }
    
    ~List() {
        clear();
        memory::SimpleAlloc<Node>::Delete(_head);
    }
    
    bool empty() const { return _head->_next == _head; }
    size_t size() const { return _size; }
    
    bool pushBack(const T &data) {
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
    
    bool pushFront(const T &data) {
        Node *n = memory::SimpleAlloc<Node>::New(data);
        if (!n) return false;
        
        Node *head = _head->_next;
        n->_next = head;
        head->_prev = n;
        n->_prev = _head;
        _head->_next = n;
        ++_size;
        
        return true;
    }
    
    T &front() const { return _head->_next->_data; }
    void popFront() { pop(_head->_next); }
    T &back() const { return _head->_prev->_data; }
    void popBack() { pop(_head->_prev); }
    
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
    
    int remove(const T &data) {
        Node *n = _head->_next;
        int cnt = 0;
        while (n != _head) {
            Node *tmp = n;
            n = n->_next;
            if (tmp->_data == data) {
                Node *prev = tmp->_prev;
                Node *next = tmp->_next;
                prev->_next = next;
                next->_prev = prev;
                memory::SimpleAlloc<Node>::Delete(tmp);
                --_size;
                ++cnt;
            }
        }
        
        return cnt;
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
    struct Node;
    void pop(Node *n) {
        Node *prev = n->_prev;
        Node *next = n->_next;
        prev->_next = next;
        next->_prev = prev;
        memory::SimpleAlloc<Node>::Delete(n);
        --_size;
    }
    
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

template <typename T>
class Stack {
public:
    bool push(const T &data) { return _list.pushFront(data); }
    
    T pop() {
        T data = _list.front();
        _list.popFront();
        return data;
    }
    
    bool empty() const { return _list.empty(); }
    size_t size() const { return _list.size(); }
    
private:
    List<T> _list;
};


}

#endif /* __UTIL_LOCK_FREE_LIST__ */
