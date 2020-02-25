#ifndef __CLOSURE_BIND_HELPER_H__
#define __CLOSURE_BIND_HELPER_H__

#include "util/template_util.h"
#include "util/weak_ptr.h"

namespace closure {
namespace internal {

using util::TrueType;
using util::FalseType;
using util::YesType;
using util::NoType;
using util::ScopedRef;
using util::WeakPtr;
using util::IsPointer;
using util::IsConvertible;

template <typename T>
class ConstRefWrapper {
public:
    explicit ConstRefWrapper(const T& o) : _ptr(&o) {}
    const T& get() const { return *_ptr; }
private:
    const T* _ptr;
};

template <typename T>
class UnretainedWrapper {
public:
    explicit UnretainedWrapper(T* o) : _ptr(o) {}
    T* get() const { return _ptr; }
private:
    T* _ptr;
};

template <typename T>
class OwnedWrapper {
public:
    explicit OwnedWrapper(T* o) : _ptr(o) {}
    ~OwnedWrapper() { delete _ptr; }
    T* get() const { return _ptr; }
    OwnedWrapper(const OwnedWrapper& other) {
        _ptr = other._ptr;
        other._ptr = NULL;
    }

private:
    mutable T* _ptr;
};

template <typename T>
class PassedWrapper {
public:
    explicit PassedWrapper(T scoper) : _isValid(true), _scoper(scoper.pass()) {}

    PassedWrapper(const PassedWrapper& other)
        : _isValid(other._isValid), _scoper(other._scoper.pass()) { }
    
    T pass() const { _isValid = false; return _scoper.pass(); }

private:
    mutable bool _isValid;
    mutable T _scoper;
};

template <typename T>
class HasIsMethodTag {
    template <typename U>
    static YesType& check(typename U::IsMethod*);

    template <typename U>
    static NoType& check(...);

public:
    enum { value = sizeof(check<T>(NULL)) == sizeof(YesType) };
};

template <typename T>
struct UnwrapTraits {
    typedef const T& ForwardType;
    static ForwardType unwrap(const T& o) { return o; }
};

template <typename T>
struct UnwrapTraits<UnretainedWrapper<T> > {
    typedef T* ForwardType;
    static ForwardType unwrap(UnretainedWrapper<T> unretained) {
        return unretained.get();
    }
};

template <typename T>
struct UnwrapTraits<ConstRefWrapper<T> > {
    typedef const T& ForwardType;
    static ForwardType unwrap(ConstRefWrapper<T> constRef) {
        return constRef.get();
    }
};

template <typename T>
struct UnwrapTraits<ScopedRef<T> > {
    typedef T* ForwardType;
    static ForwardType unwrap(const ScopedRef<T>& o) { return o.get(); }
};

template <typename T>
struct UnwrapTraits<WeakPtr<T> > {
    typedef const WeakPtr<T>& ForwardType;
    static ForwardType unwrap(const WeakPtr<T>& o) { return o; }
};

template <typename T>
struct UnwrapTraits<OwnedWrapper<T> > {
    typedef T* ForwardType;
    static ForwardType unwrap(const OwnedWrapper<T>& o) {
        return o.get();
    }
};

template <typename T>
struct UnwrapTraits<PassedWrapper<T> > {
    typedef T ForwardType;
    static T unwrap(PassedWrapper<T>& o) {
        return o.pass();
    }
};

template <bool is_method, typename T>
struct MaybeRefcount;

template <typename T>
struct MaybeRefcount<false, T> {
    static void incRef(const T&) {}
    static void decRef(const T&) {}
};

template <typename T, int n>
struct MaybeRefcount<false, T[n]> {
    static void incRef(const T*) {}
    static void decRef(const T*) {}
};

template <typename T>
struct MaybeRefcount<true, T> {
    static void incRef(const T&) {}
    static void decRef(const T&) {}
};

template <typename T>
struct MaybeRefcount<true, T*> {
    static void incRef(T*) {}
    static void decRef(T*) {}
};

template <typename T>
struct MaybeRefcount<true, ScopedRef<T> > {
    static void incRef(const ScopedRef<T>&) {}
    static void decRef(const ScopedRef<T>&) {}
};

template <typename T>
struct MaybeRefcount<true, const T*> {
    static void incRef(const T*) {}
    static void decRef(const T*) {}
};

template <bool IsMethod, typename P1> struct IsWeakMethod : FalseType {};
template <typename T> struct IsWeakMethod<true, WeakPtr<T> > : TrueType {};
template <typename T> struct IsWeakMethod<true, ConstRefWrapper<WeakPtr<T> > > : TrueType {};

template <typename T>
struct NeedsScopedRefptrButGetsRawPtr {
    enum {
        value = (IsPointer<T>::value &&
            (IsConvertible<T, util::RefCountedBase*>::value ||
            IsConvertible<T, util::RefCountedThreadSafeBase*>::value))
    };
};

} /* namespace internal */
} /* namespace closure */

#endif /* __CLOSURE_BIND_HELPER_H__ */
