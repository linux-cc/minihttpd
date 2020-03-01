#ifndef __UTIL_SCOPED_PTR_H__
#define __UTIL_SCOPED_PTR_H__

#include "util/template_util.h"
#include "memory/simple_alloc.h"

namespace util {
    
class RefCountedBase;
class RefCountedThreadSafeBase;

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() {}
    template <typename U>
    DefaultDeleter(const DefaultDeleter<U>&) {
        COMPILE_ASSERT((IsConvertible<U*, T*>::value), U_ptr_must_implicitly_convert_to_T_ptr);
    }
    inline void operator()(T* ptr) const {
        memory::SimpleAlloc<T>::Delete(ptr);
    }
};

template <typename T> struct IsNotRefCounted {
    enum {
        value = !IsConvertible<T*, RefCountedBase*>::value &&
            !IsConvertible<T*, RefCountedThreadSafeBase*>:: value
    };
};

template <typename T, typename Deleter = DefaultDeleter<T> >
class ScopedPtr {
    struct Data : public Deleter {
        explicit Data(T *p) : ptr(p) {}
        Data(T *p, const Deleter& other) : Deleter(other), ptr(p) {}
        T *ptr;
    };
    typedef Data ScopedPtr::*Testable;
    COMPILE_ASSERT(IsNotRefCounted<T>::value, T_is_refcounted_type_and_needs_scoped_refptr);
    struct RValue {
        explicit RValue(ScopedPtr* obj) : object(obj) {}
        ScopedPtr* object;
    };
public:
    explicit ScopedPtr(T *p = NULL): _data(p) {}
    ScopedPtr(T *p, const Deleter &d): _data(p, d) {}
    template <typename U, typename V>
    ScopedPtr(ScopedPtr<U, V> other): _data(other.release(), other.getDeleter()) {
        COMPILE_ASSERT(!IsArray<U>::value, U_cannot_be_an_array);
    }
    ScopedPtr(const RValue& rvalue) : _data(rvalue.object->_data) {}
    ~ScopedPtr() {
        if (_data.ptr != NULL) {
            static_cast<Deleter&>(_data)(_data.ptr);
        }
    }
    
    template <typename U, typename V>
    ScopedPtr &operator=(ScopedPtr<U, V> other) {
        COMPILE_ASSERT(!IsArray<U>::value, U_cannot_be_an_array);
        reset(other.release());
        getDeleter() = other.getDeleter();
        return *this;
    }
    
    void reset(T *p = NULL) {
        if (p == _data.ptr) {
            return;
        }
        
        T *old = _data.ptr;
        if (old != NULL) {
            static_cast<Deleter&>(_data)(old);
        }
        _data.ptr = p;
    }
    
    T &operator*() const { return *_data.ptr; }
    T *operator->() const  { return _data.ptr; }
    T *get() const { return _data.ptr; }
    
    Deleter &getDeleter() { return _data; }
    const Deleter &getDeleter() const { return _data; }
    
    operator Testable() const { return _data.ptr ? &ScopedPtr::_data : NULL; }
    bool operator==(const T *p) const { return _data.ptr == p; }
    bool operator!=(const T *p) const { return _data.ptr != p; }
    void swap(ScopedPtr& p2) { }
    
    T *release() {
        T *oldPtr = _data.ptr;
        _data.ptr = NULL;
        return oldPtr;
    }
    
    template <typename PassAsType>
    ScopedPtr<PassAsType> passAs() {
        return ScopedPtr<PassAsType>(pass());
    }
    
    operator RValue() { return RValue(this); }
    ScopedPtr pass() { return ScopedPtr(RValue(this)); }
    
private:
    Data _data;

    template <typename U, typename V> friend class ScopedPtr;
    explicit ScopedPtr(int disallowConstructionFromNull);
    template <typename U> bool operator==(ScopedPtr<U> const& p2) const;
    template <typename U> bool operator!=(ScopedPtr<U> const& p2) const;
};

template <typename T, typename Deleter>
void swap(ScopedPtr<T, Deleter> &p1, ScopedPtr<T, Deleter> &p2) {
    p1.swap(p2);
}

template <typename T, typename Deleter>
bool operator==(T *p1, const ScopedPtr<T, Deleter> &p2) {
    return p1 == p2.get();
}

template <typename T, typename Deleter>
bool operator!=(T *p1, const ScopedPtr<T, Deleter> &p2) {
    return p1 != p2.get();
}

template <typename T>
ScopedPtr<T> makeScopedPtr(T *ptr) {
    return ScopedPtr<T>(ptr);
}

} /* namespace util */
#endif  /* __UTIL_SCOPED_PTR_H__ */
