#ifndef __UTIL_SET_H__
#define __UTIL_SET_H__

#include "util/rbtree.h"

namespace util {

template <typename T>
class Set {
    template <typename Value>
    struct Identity {
        const Value &operator()(const Value &data) const { return data; }
    };
public:
    typedef typename RBTree<T, T, Identity<T> >::Iterator Iterator;
    
    Iterator find(const T &key) const { _tree.find(key); }
    Iterator begin() { return _tree.begin(); }
    Iterator end() { return _tree.end(); }
    bool empty() const { return _tree.empty(); }
    size_t size() const { return _tree.size(); }
    
    Iterator insert(const T &value) { return _tree.insert(value, true); }
    void clear() { _tree.clear(); }
    void erase(const Iterator &it) { _tree.erase(it); }
    void erase(const T &key) { _tree.erase(key); }
    
private:
    RBTree<T, T, Identity<T> > _tree;
};

}

#endif /* __UTIL_SET_H__ */
