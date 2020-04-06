#ifndef __UTIL_RBTREE_H__
#define __UTIL_RBTREE_H__

#include "memory/simple_alloc.h"

namespace util {

template <typename Key, typename Value, typename KeyOfValue>
class RBTree {
    struct Node;
public:
    class Iterator;
    
    RBTree() { _header = memory::SimpleAlloc<Node>::New(); initial(); }
    ~RBTree() { clear(); memory::SimpleAlloc<Node>::Delete(_header); }
    
    Iterator find(const Key &key) const { return find(root(), key); }
    Iterator begin() { return leftMost(); }
    Iterator end() { return _header; }
    bool empty() const { return _size == 0; }
    size_t size() const { return _size; }
    
    Iterator insert(const Value &value, bool unique) {
        Node *n = insert(value, root(), _header, unique);
        root()->_color = BLACK;
        ++_size;
        return n;
    }
    
    void clear() { erase(root()); initial(); }
    void erase(const Iterator &it) { erase(*it); }
    void erase(const Key &key) {
        root() = remove(root(), key);
        if (root()) {
            root()->_color = BLACK;
        }
        --_size;
    }
    
private:
    void initial() {
        _size = 0;
        root() = 0;
        leftMost() = _header;
        rightMost() = _header;
    }
    
    Node *insert(const Value &value, Node *&n, Node *p, bool unique) {
        const Key &key = KeyOfValue()(value);
        if (!n) {
            return n = getNode(key, value, p);
        }

        if (!(key < n->key()) && !(n->key() < key)) {
            if (!unique) {
                n->_value = value;
            }
            return n;
        }
        
        Node *_n = insert(value, key < n->key() ? n->_left : n->_right, n, unique);
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
        if (key < n->key()) {
            if (!isRed(n->_left) && !isRed(n->_left->_left)) {
                n = moveRedLeft(n);
            }
            n->_left = remove(n->_left, key);
        } else {
            if (isRed(n->_left)) {
                n = rotateRight(n);
            }
            if (!(n->key() < key) && !n->_right) {
                removeNode(n);
                return 0;
            }
            if (!isRed(n->_right) && !isRed(n->_right->_left)) {
                n = moveRedRight(n);
            }
            if (n->key() < key) {
                n->_right = remove(n->_right, key);
            } else {
                n->_value = leftMost(n->_right)->_value;
                n->_right = removeMin(n->_right);
            }
        }
        
        return fixUp(n);
    }
    
    void erase(Node *n) {
        if (!n) {
            return;
        }
        erase(n->_left);
        erase(n->_right);
        memory::SimpleAlloc<Node>::Delete(n);
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
            removeNode(n);
            return 0;
        }
        if (!isRed(n->_left) && !isRed(n->_left->_left)) {
            n = moveRedLeft(n);
        }
        n->_left = removeMin(n->_left);
        
        return fixUp(n);
    }
    
    void removeNode(Node *n) {
        if (n == leftMost()) {
            leftMost() = n->_parent;
        } else if (n == rightMost()) {
            rightMost() = n->_parent;
        }
        memory::SimpleAlloc<Node>::Delete(n);
    }
    
    Node *getNode(const Key &key, const Value &value, Node *p) {
        Node *n = memory::SimpleAlloc<Node>::New(p, value);
        if (p == _header) {
            root() = leftMost() = rightMost() = n;
        } else if (key < p->key() && p == leftMost()) {
            leftMost() = n;
        } else if (p->key() < key && p == rightMost()) {
            rightMost() = n;
        }
        
        return n;
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
        if (x->_left) {
            x->_left->_parent = y;
        }
        x->_left = y;
        
        return fixRorate(x, y);
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
        if (x->_right) {
            x->_right->_parent = y;
        }
        x->_right = y;
        
        return fixRorate(x, y);
    }
    
    Node *fixRorate(Node *x, Node *y) {
        if (y == root()) {
            root() = x;
        } else if (y == y->_parent->_left) {
            y->_parent->_left = x;
        } else {
            y->_parent->_right = x;
        }
        x->_color = y->_color;
        y->_color = RED;
        x->_parent = y->_parent;
        y->_parent = x;
        
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
    Node *&leftMost() const { return _header->_left; }
    Node *&rightMost() const { return _header->_right; }
    Node *&root() const { return _header->_parent; }
    bool isRed(Node *n) const { return n && n->_color == RED; }
    
    Node *_header;
    size_t _size;
    enum Color {
        RED,
        BLACK,
    };
};
template <typename Key, typename Value, typename KeyOfValue>
struct RBTree<Key, Value, KeyOfValue>::Node {
    Node *_parent;
    Node *_left;
    Node *_right;
    bool _color;
    Value _value;
    const Key &key() const { return KeyOfValue()(_value); }
    Node(): _parent(0), _left(0), _right(0), _color(RED) {}
    Node(Node *parent, const Value &value): _parent(parent), _left(0), _right(0), _color(RED), _value(value) {}
};

template <typename Key, typename Value, typename KeyOfValue>
class RBTree<Key, Value, KeyOfValue>::Iterator {
public:
    Iterator(): _n(0) {}
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
    
    Value &operator*() const { return _n->_value; }
    Value *operator->() const { return &(operator*()); }
    bool operator==(const Iterator &other) const { return _n == other._n; }
    bool operator!=(const Iterator &other) const { return _n != other._n; }
    bool color() const { return _n->_color; }
    
private:
    Iterator(Node *n): _n(n) {}
    
    void increment() {
        if (_n->_right) {
            _n = _n->_right;
            while (_n->_left) {
                _n = _n->_left;
            }
        } else {
            Node *p = _n->_parent;
            while (_n == p->_right) {
                _n = p;
                p = p->_parent;
            }
            if (_n->_right != p) {
                _n = p;
            }
        }
    }
    
    void decrement() {
        if (_n->_color == RBTree::RED && _n->_parent->_parent == _n) {
            _n = _n->_right;
        } else if (_n->_left) {
            Node *l = _n->_left;
            while (l->_right) {
                l = l->_right;
            }
            _n = l;
        } else {
            Node *p = _n->_parent;
            while (_n == p->_left) {
                _n = p;
                p = p->_parent;
            }
            _n = p;
        }
    }
    
    Node *_n;
    friend class RBTree;
};

} /* namespace util */

#endif
