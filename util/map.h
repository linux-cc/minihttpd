#ifndef __UTIL_MAP_H__
#define __UTIL_MAP_H__

#include "util/rbtree.h"

namespace util {

template <class T1, class T2>
struct Pair {
    typedef T1 FirstType;
    typedef T2 SecondType;
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
    template <typename Pair>
    struct SelectKey {
        const typename Pair::FirstType &operator()(const Pair &pair) const { return pair.first; }
    };
    
public:
    typedef Pair<Key, Value> ValueType;
    typedef typename RBTree<Key, ValueType, SelectKey<ValueType> >::Iterator Iterator;
    
    Iterator find(const Key &key) const { _tree.find(key); }
    Iterator begin() { return _tree.begin(); }
    Iterator end() { return _tree.end(); }
    bool empty() const { return _tree.empty(); }
    size_t size() const { return _tree.size(); }
    
    Value &operator[](const Key &key) { return insert(makePair(key, Value()))->second; }
    Iterator insert(const ValueType &value) { return _tree.insert(value, false); }
    void clear() { _tree.clear(); }
    void erase(const Iterator &it) { _tree.erase(it); }
    void erase(const Key &key) { _tree.erase(key); }
    
private:
    RBTree<Key, ValueType, SelectKey<ValueType> > _tree;
};

}

#endif /* __UTIL_MAP_H__ */
