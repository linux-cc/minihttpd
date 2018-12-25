#ifndef __UTIL_SCOPED_PTR_H__
#define __UTIL_SCOPED_PTR_H__

#include <algorithm>
#include "util/util.h"

namespace util {
    
class RefCountedBase;
class RefCountedThreadSafeBase;

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() {}
    template <typename U> DefaultDeleter(const DefaultDeleter<U>&) {
        enum { TMustBeComplete = sizeof(T) };
        enum { UMustBeComplete = sizeof(U) };
        COMPILE_ASSERT((IsConvertible<U*, T*>::value), U_ptr_must_implicitly_convert_to_T_ptr);
    }
    inline void operator()(T* ptr) const {
        enum { TypeMustBeComplete = sizeof(T) };
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {
    inline void operator()(T* ptr) const {
        enum { TypeMustBeComplete = sizeof(T) };
        delete[] ptr;
    }
    
private:
    template <typename U> void operator()(U* array) const;
};

template <typename T, int n>
struct DefaultDeleter<T[n]> {
    COMPILE_ASSERT(sizeof(T) == -1, do_not_use_array_with_size_as_type);
};

struct FreeDeleter {
    inline void operator()(void* ptr) const {
        free(ptr);
    }
};

template <typename T> struct IsNotRefCounted {
    enum {
        value = !IsConvertible<T*, RefCountedBase*>::value &&
            !IsConvertible<T*, RefCountedThreadSafeBase*>:: value
    };
};

template <typename T, typename Deleter>
class ScopedPtrImpl {
public:
    explicit ScopedPtrImpl(T* p) : _data(p) { }

    ScopedPtrImpl(T* p, const Deleter& d) : _data(p, d) {}

    template <typename U, typename V>
    ScopedPtrImpl(ScopedPtrImpl<U, V>* other)
        : _data(other->release(), other->getDeleter()) {
    }
    
    template <typename U, typename V>
    void takeState(ScopedPtrImpl<U, V>* other) {
        reset(other->release());
        getDeleter() = other->getDeleter();
    }
    
    ~ScopedPtrImpl() {
        if (_data.ptr != NULL) {
            static_cast<Deleter&>(_data)(_data.ptr);
        }
    }
    
    void reset(T* p) {
        if (p != NULL && p == _data.ptr) {
            abort();
        }
        
        T* old = _data.ptr;
        _data.ptr = NULL;
        if (old != NULL) {
            static_cast<Deleter&>(_data)(old);
        }
        _data.ptr = p;
    }
    
    T* get() const { return _data.ptr; }
    
    Deleter& getDeleter() { return _data; }

    const Deleter& getDeleter() const { return _data; }
    
    void swap(ScopedPtrImpl& p2) {
        using std::swap;
        swap(static_cast<Deleter&>(_data), static_cast<Deleter&>(p2._data));
        swap(_data.ptr, p2._data.ptr);
    }
    
    T* release() {
        T* old_ptr = _data.ptr;
        _data.ptr = NULL;
        return old_ptr;
    }
    
private:
    template <typename U, typename V> friend class ScopedPtrImpl;
    
    struct Data : public Deleter {
        explicit Data(T* ptr_in) : ptr(ptr_in) {}
        Data(T* ptr_in, const Deleter& other) : Deleter(other), ptr(ptr_in) {}
        T* ptr;
    };
    
    Data _data;
    
    DISALLOW_COPY_AND_ASSIGN(ScopedPtrImpl);
};

template <typename T, typename Deleter = DefaultDeleter<T> >
class ScopedPtr {
    typedef ScopedPtrImpl<T, Deleter> ScopedPtr::*Testable;

    MOVE_ONLY_TYPE_FOR_CPP_03(ScopedPtr, RValue)

    COMPILE_ASSERT(IsNotRefCounted<T>::value, T_is_refcounted_type_and_needs_scoped_refptr);

public:
    ScopedPtr() : _impl(NULL) { }
    
    explicit ScopedPtr(T* p) : _impl(p) { }
    
    ScopedPtr(T* p, const Deleter& d) : _impl(p, d) { }
    
    template <typename U, typename V>
    ScopedPtr(ScopedPtr<U, V> other) : _impl(&other._impl) {
        COMPILE_ASSERT(!IsArray<U>::value, U_cannot_be_an_array);
    }
    
    ScopedPtr(const RValue& rvalue) : _impl(&rvalue.object->_impl) { }
    
    template <typename U, typename V>
    ScopedPtr& operator=(ScopedPtr<U, V> rhs) {
        COMPILE_ASSERT(!IsArray<U>::value, U_cannot_be_an_array);
        _impl.takeState(&rhs._impl);
        return *this;
    }
    
    void reset(T* p = NULL) { _impl.reset(p); }
    
    T& operator*() const {
        assert(_impl.get() != NULL);
        return *_impl.get();
    }

    T* operator->() const  {
        assert(_impl.get() != NULL);
        return _impl.get();
    }

    T* get() const { return _impl.get(); }
    
    Deleter& getDeleter() { return _impl.getDeleter(); }

    const Deleter& getDeleter() const { return _impl.getDeleter(); }
    
    operator Testable() const { return _impl.get() ? &ScopedPtr::_impl : NULL; }
    
    bool operator==(const T* p) const { return _impl.get() == p; }

    bool operator!=(const T* p) const { return _impl.get() != p; }
    
    void swap(ScopedPtr& p2) {
        _impl.swap(p2._impl);
    }
    
    T* release() {
        return _impl.release();
    }
    
    template <typename PassAsType>
    ScopedPtr<PassAsType> passAs() {
        return ScopedPtr<PassAsType>(pass());
    }
    
private:
    template <typename U, typename V> friend class ScopedPtr;
    ScopedPtrImpl<T, Deleter> _impl;
    
    explicit ScopedPtr(int disallowConstructionFromNull);
    template <typename U> bool operator==(ScopedPtr<U> const& p2) const;
    template <typename U> bool operator!=(ScopedPtr<U> const& p2) const;
};

template <typename T, typename Deleter>
class ScopedPtr<T[], Deleter> {
    typedef ScopedPtrImpl<T, Deleter> ScopedPtr::*Testable;

    MOVE_ONLY_TYPE_FOR_CPP_03(ScopedPtr, RValue)
    
public:
    ScopedPtr() : _impl(NULL) { }
    
    explicit ScopedPtr(T* array) : _impl(array) { }
    
    ScopedPtr(const RValue& rvalue) : _impl(&rvalue.object->_impl) { }
    
    ScopedPtr& operator=(RValue rhs) {
        _impl.takeState(&rhs.object->_impl);
        return *this;
    }
    
    void reset(T* array = NULL) { _impl.reset(array); }
    
    T& operator[](size_t i) const {
        assert(_impl.get() != NULL);
        return _impl.get()[i];
    }

    T* get() const { return _impl.get(); }
    
    Deleter& getdeleter() { return _impl.getDeleter(); }
    const Deleter& getDeleter() const { return _impl.getDeleter(); }
    
public:
    operator Testable() const { return _impl.get() ? &ScopedPtr::_impl : NULL; }
    
    bool operator==(T* array) const { return _impl.get() == array; }

    bool operator!=(T* array) const { return _impl.get() != array; }
    
    void swap(ScopedPtr& p2) {
        _impl.swap(p2._impl);
    }
    
    T* release() {
        return _impl.release();
    }

private:
    enum { TypeMustBeComplete = sizeof(T) };
    
    ScopedPtrImpl<T, Deleter> _impl;
    
    template <typename U> explicit ScopedPtr(U* array);
    explicit ScopedPtr(int disallowConstructionFromNull);
    template <typename U> void reset(U* array);
    void reset(int disallowResetFromNull);
    template <typename U> bool operator==(ScopedPtr<U> const& p2) const;
    template <typename U> bool operator!=(ScopedPtr<U> const& p2) const;
};

template <typename T, typename Deleter>
void swap(ScopedPtr<T, Deleter>& p1, ScopedPtr<T, Deleter>& p2) {
    p1.swap(p2);
}

template <typename T, typename Deleter>
bool operator==(T* p1, const ScopedPtr<T, Deleter>& p2) {
    return p1 == p2.get();
}

template <typename T, typename Deleter>
bool operator!=(T* p1, const ScopedPtr<T, Deleter>& p2) {
    return p1 != p2.get();
}

template <typename T>
ScopedPtr<T> makeScopedPtr(T* ptr) {
    return ScopedPtr<T>(ptr);
}

} /* namespace util */
#endif  /* __UTIL_SCOPED_PTR_H__ */
