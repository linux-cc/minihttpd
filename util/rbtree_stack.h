#ifndef __UTIL_RBTREE_STACK_H__
#define __UTIL_RBTREE_STACK_H__

#include "util/scoped_ref.h"
#include "util/list.h"

namespace util {

template <typename Key, typename Value>
class RBTreeT {
    struct Node;
public:
    class Iterator;
    
    RBTreeT(): _root(0) {}
    
    Iterator find(const Key &key) const { return find(_root, key); }
    Iterator begin() { return Iterator(_root, true); }
    Iterator end() { return Iterator(0); }
    
    Value &operator[](const Key &key) { return insert(key, Value()).value(); }
    Iterator insert(const Key &key, const Value &value) {
        Node *n = insert(key, value, _root);
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
    Node *insert(const Key &key, const Value &value, Node *&n) {
        if (!n) {
            return n = new Node(key, value);
        }

        Node *_n;
        if (key < n->_key) {
            _n = insert(key, value, n->_left);
        } else if (n->_key < key) {
            _n = insert(key, value, n->_right);
        } else {
            (_n = n)->_value = value;
        }
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
                return 0;
            }
            if (!isRed(n->_right) && !isRed(n->_right->_left)) {
                n = moveRedRight(n);
            }
            if (n->_key < key) {
                n->_right = remove(n->_right, key);
            } else {
                n->_key = leftMost(n->_right)->_key;
                n->_value = find(n->_right, n->_key)->_value;
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
template <typename Key, typename Value>
struct RBTreeT<Key, Value>::Node {
    Node *_left;
    Node *_right;
    bool _color;
    Key _key;
    Value _value;
    Node(): _left(0), _right(0), _color(RED) {}
    Node(const Key &key, const Value &value):
    _left(0), _right(0), _color(RED), _key(key), _value(value) {}
};

template <typename Key, typename Value>
class RBTreeT<Key, Value>::Iterator {
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
    
    bool operator==(const Iterator &other) const { return _n == other._n; }
    bool operator!=(const Iterator &other) const { return _n != other._n; }
    
    Key key() { return _n->_key; }
    Value &value() { return _n->_value; }
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
