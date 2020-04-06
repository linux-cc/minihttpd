#ifndef __UTIL_RBTREE_H__
#define __UTIL_RBTREE_H__

#include "memory/simple_alloc.h"

namespace util {

template <typename Key>
class RBTree {
    struct Node;
public:
    class Iterator;
    
    RBTree() {
        _header = memory::SimpleAlloc<Node>::New();
        root() = 0;
        leftMost() = _header;
        rightMost() = _header;
    }
    ~RBTree() { memory::SimpleAlloc<Node>::Delete(_header); }
    
    Iterator find(const Key &key) const { return find(root(), key); }
    Iterator begin() { return leftMost(); }
    Iterator end() { return _header; }
    
    Iterator insert(const Key &key, bool unique = false) {
        Node *n = insert(key, root(), _header, unique);
        root()->_color = BLACK;
        return n;
    }
    
    void erase(const Iterator &it) { erase(it.key()); }
    void erase(const Key &key) {
        root() = remove(root(), key);
        if (root()) {
            root()->_color = BLACK;
        }
    }
    
private:
    Node *insert(const Key &key, Node *&n, Node *p, bool unique) {
        if (!n) {
            return n = getNode(key, p);
        }

        if (!(key < n->_key) && !(n->_key < key)) {
            if (!unique) {
                n->_key = key;
            }
            return n;
        }
        
        Node *_n = insert(key, key < n->_key ? n->_left : n->_right, n, unique);
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
                removeNode(n);
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
    
    Node *getNode(const Key &key, Node *p) {
        Node *n = memory::SimpleAlloc<Node>::New(p, key);
        if (p == _header) {
            root() = leftMost() = rightMost() = n;
        } else if (key < p->_key && p == leftMost()) {
            leftMost() = n;
        } else if (p->_key < key && p == rightMost()) {
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
    enum Color {
        RED,
        BLACK,
    };
};
template <typename Key>
struct RBTree<Key>::Node {
    Node *_parent;
    Node *_left;
    Node *_right;
    bool _color;
    Key _key;
    Node(): _parent(0), _left(0), _right(0), _color(RED) {}
    Node(Node *parent, const Key &key): _parent(parent), _left(0), _right(0), _color(RED), _key(key) {}
};

template <typename Key>
class RBTree<Key>::Iterator {
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
    
    Key &operator*() const { return _n->_key; }
    Key *operator->() const { return &(operator*()); }
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
