#ifndef __UTIL_WEAK_PTR_H__
#define __UTIL_WEAK_PTR_H__

#include "util/scoped_ref.h"
#include "memory/simple_alloc.h"

namespace util {

template <typename T> class SupportsWeakPtr;
template <typename T> class WeakPtr;

namespace internal {

class WeakRef {
public:
    class Flag : public RefCountedThreadSafe<Flag> {
    public:
        Flag() : _isValid(true) {}
        
        void invalidate() { _isValid = false; }

        bool isValid() const { return _isValid; }
        
    private:
        ~Flag() {}
        
        bool _isValid;

        friend class memory::SimpleAlloc<Flag>;
    };
    
    WeakRef() {}
    
    explicit WeakRef(const Flag* flag) : _flag(flag) {}

    bool isValid() const { return _flag.get() && _flag->isValid(); }
    
private:
    ScopedRef<const Flag> _flag;
};

class WeakRefOwner {
public:
    ~WeakRefOwner() { invalidate(); }
    
    WeakRef getRef() const {
        if (!hasRef()) {
            _flag = new WeakRef::Flag();
        }
        return WeakRef(_flag);
    }
    
    bool hasRef() const {
        return _flag.get() && _flag->hasRef();
    }
    
    void invalidate() {
        if (_flag.get()) {
            _flag->invalidate();
            _flag = NULL;
        }
    }
    
private:
    mutable ScopedRef<WeakRef::Flag> _flag;
};

class WeakPtrBase {
protected:
    explicit WeakPtrBase(const WeakRef& ref) : _ref(ref) {}
    
    WeakRef _ref;
};

class SupportsWeakPtrBase {
public:
    template<typename Derived>
    static WeakPtr<Derived> staticAsWeakPtr(Derived* t) {
        typedef IsConvertible<Derived, SupportsWeakPtrBase&> convertible;
        COMPILE_ASSERT(convertible::value, AsWeakPtr_argument_inherits_from_SupportsWeakPtr);
        return asWeakPtrImpl<Derived>(t, *t);
    }
    
private:
    template <typename Derived, typename Base>
    static WeakPtr<Derived> asWeakPtrImpl(Derived* t, const SupportsWeakPtr<Base>&) {
        WeakPtr<Base> ptr = t->Base::asWeakPtr();
        return WeakPtr<Derived>(ptr._ref, static_cast<Derived*>(ptr._ptr));
    }
};
    
} /* namespace internal */

template <typename T> class WeakPtrFactory;

template <typename T>
class WeakPtr : public internal::WeakPtrBase {
    typedef T* WeakPtr::*Testable;
public:
    // Allow conversion from U to T provided U "is a" T. Note that this
    // is separate from the (implicit) copy constructor.
    template <typename U>
    WeakPtr(const WeakPtr<U>& other) : WeakPtrBase(other), _ptr(other._ptr) { }
    
    T* get() const { return _ref.isValid() ? _ptr : NULL; }
    
    T& operator*() const { return *_ptr; }

    T* operator->() const { return _ptr; }
    
    operator Testable() const { return get() ? &WeakPtr::_ptr : NULL; }
    
    void reset() {
        _ref = internal::WeakRef();
        _ptr = NULL;
    }
    
private:
    template <class U> bool operator==(WeakPtr<U> const&) const;
    template <class U> bool operator!=(WeakPtr<U> const&) const;
    
    friend class internal::SupportsWeakPtrBase;
    template <typename U> friend class WeakPtr;
    friend class SupportsWeakPtr<T>;
    friend class WeakPtrFactory<T>;
    
    WeakPtr(const internal::WeakRef& ref, T* ptr)
        : WeakPtrBase(ref), _ptr(ptr) { }
    
    T* _ptr;
};

template <class T>
class WeakPtrFactory {
public:
    explicit WeakPtrFactory(T* ptr) : _ptr(ptr) { }
    
    ~WeakPtrFactory() { _ptr = NULL; }
    
    WeakPtr<T> getWeakPtr() {
        return WeakPtr<T>(_weakRefOwner.getRef(), _ptr);
    }
    
    void invalidate() {
        _weakRefOwner.invalidate();
    }
    
    bool hasWeakPtr() const {
        return _weakRefOwner.hasRef();
    }
    
private:
    internal::WeakRefOwner _weakRefOwner;
    T* _ptr;
    DISALLOW_IMPLICIT_CONSTRUCTORS(WeakPtrFactory);
};

template <class T>
class SupportsWeakPtr : public internal::SupportsWeakPtrBase {
public:
    SupportsWeakPtr() {}
    
    WeakPtr<T> asWeakPtr() {
        return WeakPtr<T>(_weakRefOwner.getRef(), static_cast<T*>(this));
    }
    
protected:
    ~SupportsWeakPtr() {}
    
private:
    internal::WeakRefOwner _weakRefOwner;
    DISALLOW_COPY_AND_ASSIGN(SupportsWeakPtr);
};

template <typename Derived>
WeakPtr<Derived> AsWeakPtr(Derived* t) {
    return internal::SupportsWeakPtrBase::staticAsWeakPtr<Derived>(t);
}

} /* namespace util */

#endif  /* __UTIL_WEAK_PTR_H_ */
