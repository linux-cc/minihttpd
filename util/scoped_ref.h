#ifndef __UTIL_REF_COUNTED_H__
#define __UTIL_REF_COUNTED_H__

#include "util/atomic.h"
#include "memory/simple_alloc.h"

namespace util {

class RefCountedBase {
public:
    bool hasRef() const { return _count > 1; }
    
    int refConut() const { return _count; }

protected:
    RefCountedBase() : _count(0) {}

    ~RefCountedBase() {}

    void incRef() const { ++_count; }

    bool decRef() const { return --_count == 0; }

private:
    mutable int _count;

    DISALLOW_COPY_AND_ASSIGN(RefCountedBase);
};

class RefCountedThreadSafeBase {
public:
    bool hasRef() const { return _count > 1; }

    int refConut() const { return _count; }
    
protected:
    RefCountedThreadSafeBase() : _count(0) {}

    ~RefCountedThreadSafeBase() {}

    void incRef() const { atomicInc(&_count); }

    bool decRef() const { return atomicDec(&_count) == 0; }

private:
    mutable int _count;

    DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafeBase);
};

template <class T, typename Traits> class RefCounted;
template<typename T>
struct DefaultRefCountedTraits {
    static void destruct(const T* x) { RefCounted<T, DefaultRefCountedTraits>::deleteInternal(x); }
};

template <typename T, typename Traits = DefaultRefCountedTraits<T> >
class RefCounted : public RefCountedBase {
public:
    RefCounted() {}

    void incRef() const { RefCountedBase::incRef(); }

    void decRef() const {
        if (RefCountedBase::decRef()) {
            Traits::destruct(static_cast<const T*>(this));
        }
    }

protected:
    ~RefCounted() {}
    
    static void deleteInternal(const T *x) { memory::SimpleAlloc<T>::Delete(x); }
    
    friend struct DefaultRefCountedTraits<T>;
    
    DISALLOW_COPY_AND_ASSIGN(RefCounted);
};

template <class T, typename Traits> class RefCountedThreadSafe;
template<typename T>
struct DefaultRefCountedThreadSafeTraits {
    static void destruct(const T* x) { RefCountedThreadSafe<T, DefaultRefCountedThreadSafeTraits>::deleteInternal(x); }
};

template <class T, typename Traits = DefaultRefCountedThreadSafeTraits<T> >
class RefCountedThreadSafe : public RefCountedThreadSafeBase {
public:
    RefCountedThreadSafe() {}

    void incRef() const { RefCountedThreadSafeBase::incRef(); }

    void decRef() const {
        if (RefCountedThreadSafeBase::decRef()) {
            Traits::destruct(static_cast<const T*>(this));
        }
    }

protected:
    ~RefCountedThreadSafe() {}

    static void deleteInternal(const T *x) { memory::SimpleAlloc<T>::Delete(x); }
    
    friend struct DefaultRefCountedThreadSafeTraits<T>;
    
    DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafe);
};

template <class T>
class ScopedRef {
public:
    ScopedRef(T *p = NULL) : _ptr(p) {
        if (_ptr) {
            _ptr->incRef();
        }
    }

    ScopedRef(const ScopedRef &r) : _ptr(r._ptr) {
        if (_ptr) {
            _ptr->incRef();
        }
    }

    template <typename U>
    ScopedRef(const ScopedRef<U> &r) : _ptr(r.get()) {
        if (_ptr) {
            _ptr->incRef();
        }
    }

    ~ScopedRef() {
        if (_ptr) {
            _ptr->decRef();
        }
    }

    T *get() const { return _ptr; }

    operator T *() const { return _ptr; }

    T *operator->() const {
        return _ptr;
    }
    
    T &operator *() const {
        return *_ptr;
    }

    ScopedRef &operator=(T* p) {
        // AddRef first so that self assignment should work
        if (p) {
            p->incRef();
        }
        T* old = _ptr;
        _ptr = p;
        if (old) {
            old->decRef();
        }

        return *this;
    }

    ScopedRef &operator=(const ScopedRef &r) {
        return *this = r._ptr;
    }

    template <typename U>
    ScopedRef &operator=(const ScopedRef<U> &r) {
        return *this = r.get();
    }

    void swap(T **pp) {
        T *p = _ptr;
        _ptr = *pp;
        *pp = p;
    }

    void swap(ScopedRef &r) {
        swap(&r._ptr);
    }

protected:
    T* _ptr;
};

} /* namespace util */

#endif /* __UTIL_REF_COUNTED_H__ */

