#ifndef __UTIL_MAP_H__
#define __UTIL_MAP_H__

#include "util/rbtree.h"

namespace util {

template <class T1, class T2>
struct Pair {
    T1 first;
    T2 second;
    Pair(): first(T1()), second(T2()) {}
    Pair(const T1 &t1, const T2 &t2): first(t1), second(t2) {}
    bool operator<(const Pair &other) const { return first < other.first; }
    bool operator==(const Pair &other) const { return first == other.first && second == other.second; }
};

template <class T1, class T2>
inline Pair<T1, T2> makePair(const T1& p1, const T2& p2) {
    return Pair<T1, T2>(p1, p2);
}

template <typename Key, typename Value>
class Map {
    typedef Pair<Key, Value> Node;
public:
    typedef RBTree<Node>::Iterator Iterator;
    
    Iterator find(const Key &key) const { _tree.find(key); }
    Iterator begin() { return _tree.begin(); }
    Iterator end() { return _tree.end(); }
    bool empty() const { return _tree.empty(); }
    size_t size() const { return _tree.size(); }
    
    Iterator insert(const Key &key, const Value &value) { return _tree.insert(makePair(key, value), true); }
    void clear() { _tree.clear(); }
    void erase(const Iterator &it) { _tree.erase(it); }
    void erase(const Key &key) { _tree.erase(key); }
    }
    
private:
    RBTree<Node> _tree;
};

}

#endif /* __UTIL_MAP_H__ */
