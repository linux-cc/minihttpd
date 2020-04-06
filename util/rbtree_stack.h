#ifndef __UTIL_RBTREE_STACK_H__
#define __UTIL_RBTREE_STACK_H__

#include "memory/simple_alloc.h"
#include "util/list.h"

namespace util {

template <typename Key>
class RBTreeT {
    struct Node;
public:
    class Iterator;
    
    RBTreeT(): _root(0) {}
    
    Iterator find(const Key &key) const { return find(_root, key); }
    Iterator begin() { return Iterator(_root, true); }
    Iterator end() { return Iterator(0); }
    
    Iterator insert(const Key &key, bool unique = false) {
        Node *n = insert(key, _root, unique);
        _root->_color = BLACK;
        return n;
    }
    
    void erase(const Key &key) {
        _root = remove(_root, key);
        if (_root) {
            _root->_color = BLACK;
        }
    }
    
private:
    Node *insert(const Key &key, Node *&n, bool unique) {
        if (!n) {
            return n = memory::SimpleAlloc<Node>::New(key);
        }

        if (!(key < n->_key) && !(n->_key < key)) {
            if (!unique) {
                n->_key = key;
            }
            return n;
        }
        
        Node *_n = insert(key, key < n->_key ? n->_left : n->_right, unique);
        if (isRed(n->_right)) {
            n = rotateLeft(n);
        }
        if (isRed(n->_left) && isRed(n->_left->_left)) {
            n = rotateRight(n);
        }
        if (isRed(n->_left) && isRed(n->_right)) {
            colorFlip(n);
        }
        return _n;
    }
    
    Node *remove(Node *n, const Key &key) {
        if (key < n->_key) {
            if (!isRed(n->_left) && !isRed(n->_left->_left)) {
                n = moveRedLeft(n);
            }
            n->_left = remove(n->_left, key);
        } else {
            if (isRed(n->_left)) {
                n = rotateRight(n);
            }
            if (!(n->_key < key) && !n->_right) {
                memory::SimpleAlloc<Node>::Delete(n);
                return 0;
            }
            if (!isRed(n->_right) && !isRed(n->_right->_left)) {
                n = moveRedRight(n);
            }
            if (n->_key < key) {
                n->_right = remove(n->_right, key);
            } else {
                n->_key = leftMost(n->_right)->_key;
                n->_right = removeMin(n->_right);
            }
        }
        
        return fixUp(n);
    }
    
    Node *find(Node *n, const Key &key) const {
        while (n) {
            if (key < n->_key) {
                n = n->_left;
            } else if (n->_key < key) {
                n = n->_right;
            } else {
                break;
            }
        }
        
        return n;
    }
    
    Node *removeMin(Node *n) {
        if (!n->_left) {
            memory::SimpleAlloc<Node>::Delete(n);
            return 0;
        }
        if (!isRed(n->_left) && !isRed(n->_left->_left)) {
            n = moveRedLeft(n);
        }
        n->_left = removeMin(n->_left);
        
        return fixUp(n);
    }
    
    /*
            y                       x
          /    \                  /   \
        yl       x       ->      y     xr
              /     \          /   \
             xl     xr       yl     xl
    */
    Node *rotateLeft(Node *y) {
        Node *x = y->_right;
        y->_right = x->_left;
        x->_left = y;
        x->_color = y->_color;
        y->_color = RED;

        return x;
    }
    
    /*
                y                x
              /    \           /   \
            x       yr   ->   xl    y
          /   \                   /   \
        xl     xr                xr    yr
    */
    Node *rotateRight(Node *y) {
        Node *x = y->_left;
        y->_left = x->_right;
        x->_right = y;
        x->_color = y->_color;
        y->_color = RED;
        
        return x;
    }
    
    void colorFlip(Node *n) {
        n->_color = !n->_color;
        n->_left->_color = !n->_left->_color;
        n->_right->_color = !n->_right->_color;
    }
    
    Node *fixUp(Node *n) {
        if (isRed(n->_right)) {
            n = rotateLeft(n);
        }
        if (isRed(n->_left) && isRed(n->_left->_left)) {
            n = rotateRight(n);
        }
        if (isRed(n->_left) && isRed(n->_right)) {
            colorFlip(n);
        }
        
        return n;
    }
    
    Node *moveRedRight(Node *n) {
        colorFlip(n);
        if (isRed(n->_left->_left)) {
            n = rotateRight(n);
            colorFlip(n);
        }
        
        return n;
    }
    
    Node *moveRedLeft(Node *n) {
        colorFlip(n);
        if (isRed(n->_right->_left)) {
            n->_right = rotateRight(n->_right);
            n = rotateLeft(n);
            colorFlip(n);
        }
        
        return n;
    }
    
    Node *leftMost(Node *n) const {
        while (n && n->_left) {
            n = n->_left;
        }
        
        return n;
    }
    
    Node *rightMost(Node *n) const {
        while (n && n->_right) {
            n = n->_right;
        }
        
        return n;
    }

    bool isRed(Node *n) const { return n && n->_color == RED; }
    
    Node *_root;
    enum Color {
        RED,
        BLACK,
    };
};
template <typename Key>
struct RBTreeT<Key>::Node {
    Node *_left;
    Node *_right;
    bool _color;
    Key _key;
    Node(): _left(0), _right(0), _color(RED) {}
    Node(const Key &key): _left(0), _right(0), _color(RED), _key(key) {}
};

template <typename Key>
class RBTreeT<Key>::Iterator {
public:
    Iterator():_n(0) {}
    Iterator &operator++() { increment(); return *this; }
    Iterator operator++(int) {
        Iterator it = *this;
        increment();
        return it;
    }
    
    Iterator &operator--() { decrement(); return *this; }
    Iterator operator--(int) {
        Iterator it = *this;
        decrement();
        return it;
    }
    
    Key &operator*() const { return _n->_key; }
    Key *operator->() const { return &(operator*()); }
    bool operator==(const Iterator &other) const { return _n == other._n; }
    bool operator!=(const Iterator &other) const { return _n != other._n; }
    bool color() { return _n->_color; }
    
private:
    Iterator(Node *n, bool travel = false): _n(n) {
        if (travel) {
            _ref = memory::SimpleAlloc<RefData>::New();
            pushLeft(n);
            increment();
        }
    }
    
    void pushLeft(Node *n) {
        while (n) {
            _ref->_stack.push(n);
            n = n->_left;
        }
    }
    
    void increment() {
        if (_ref->_stack.empty()) {
            _n = 0;
        } else {
            _n = _ref->_stack.pop();
            pushLeft(_n->_right);
        }
    }
    
    void decrement() {

    }
    
    struct RefData : public RefCountedThreadSafe<RefData> {
        Stack<Node*> _stack;
    };
    ScopedRef<RefData> _ref;
    Node *_n;
    friend class RBTreeT;
};

}

#endif /* __UTIL_RBTREE_STACK_H__ */
