#ifndef __UTIL_REF_COUNTED_H__
#define __UTIL_REF_COUNTED_H__

#include "util/util.h"
#include "util/atomic.h"

namespace util {

class RefCountedBase {
public:
    bool hasOneRef() const { return _count == 1; }

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
    bool hasOneRef() const { return _count == 1; }

protected:
    RefCountedThreadSafeBase() : _count(0) {}

    ~RefCountedThreadSafeBase() {}

    void incRef() const { atomicInc(&_count); }

    bool decRef() const { return atomicDec(&_count) == 0; }

private:
    mutable int _count;

    DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafeBase);
};

template <typename T>
class RefCounted : public RefCountedBase {
public:
    RefCounted() {}

    void incRef() const {
        RefCountedBase::incRef();
    }

    void decRef() const {
        if (RefCountedBase::decRef()) {
            delete static_cast<const T*>(this);
        }
    }

protected:
    ~RefCounted() {}

private:
    DISALLOW_COPY_AND_ASSIGN(RefCounted<T>);
};

template <class T, typename Traits> class RefCountedThreadSafe;

template<typename T>
struct DefaultRefCountedThreadSafeTraits {
    static void Destruct(const T* x) {
        RefCountedThreadSafe<T, DefaultRefCountedThreadSafeTraits>::DeleteInternal(x);
    }
};

template <class T, typename Traits = DefaultRefCountedThreadSafeTraits<T> >
class RefCountedThreadSafe : public RefCountedThreadSafeBase {
public:
    RefCountedThreadSafe() {}

    void incRef() const {
        RefCountedThreadSafeBase::incRef();
    }

    void decRef() const {
        if (RefCountedThreadSafeBase::decRef()) {
            Traits::Destruct(static_cast<const T*>(this));
        }
    }

protected:
    ~RefCountedThreadSafe() {}

private:
    friend struct DefaultRefCountedThreadSafeTraits<T>;
    static void DeleteInternal(const T* x) { delete x; }

    DISALLOW_COPY_AND_ASSIGN(RefCountedThreadSafe);
};

template <class T>
class ScopedRefptr {
public:
    ScopedRefptr() : _ptr(NULL) { }

    ScopedRefptr(T* p) : _ptr(p) {
        if (_ptr) {
            _ptr->incRef();
        }
    }

    ScopedRefptr(const ScopedRefptr& r) : _ptr(r._ptr) {
        if (_ptr) {
            _ptr->incRef();
        }
    }

    template <typename U>
    ScopedRefptr(const ScopedRefptr<U>& r) : _ptr(r.get()) {
        if (_ptr) {
            _ptr->incRef();
        }
    }

    ~ScopedRefptr() {
        if (_ptr) {
            _ptr->decRef();
        }
    }

    T* get() const { return _ptr; }

    operator T*() const { return _ptr; }

    T* operator->() const {
        return _ptr;
    }

    ScopedRefptr<T>& operator=(T* p) {
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

    ScopedRefptr<T>& operator=(const ScopedRefptr<T>& r) {
        return *this = r._ptr;
    }

    template <typename U>
    ScopedRefptr<T>& operator=(const ScopedRefptr<U>& r) {
        return *this = r.get();
    }

    void swap(T** pp) {
        T* p = _ptr;
        _ptr = *pp;
        *pp = p;
    }

    void swap(ScopedRefptr<T>& r) {
        swap(&r._ptr);
    }

protected:
    T* _ptr;
};

template <typename T>
ScopedRefptr<T> makeScopedRefptr(T* t) {
    return ScopedRefptr<T>(t);
}

} /* namespace util */

#endif /* __UTIL_REF_COUNTED_H__ */

