#ifndef __CLOSURE_RUNNABLE_ADAPTER_H__
#define __CLOSURE_RUNNABLE_ADAPTER_H__

#include "util/util.h"
#include "closure/callback_param_traits.h"

namespace closure {
namespace internal {

using util::TrueType;

template <typename Functor>
class RunnableAdapter;

template <typename R>
class RunnableAdapter<R(*)()> {
public:
    typedef R (RunType)();
    
    explicit RunnableAdapter(R(*function)()) : _function(function) { }

    R run() { return _function(); }
    
private:
    R (*_function)();
};

template <typename R, typename T>
class RunnableAdapter<R(T::*)()> {
public:
    typedef R (RunType)(T*);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)()) : _method(method) { }

    R run(T* object) { return (object->*_method)(); }
    
private:
    R (T::*_method)();
};

template <typename R, typename T>
class RunnableAdapter<R(T::*)() const> {
public:
    typedef R (RunType)(const T*);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)() const) : _method(method) { }

    R run(const T* object) { return (object->*_method)(); }
    
private:
    R (T::*_method)() const;
};

template <typename R, typename A1>
class RunnableAdapter<R(*)(A1)> {
public:
    typedef R (RunType)(A1);
    
    explicit RunnableAdapter(R(*function)(A1)) : _function(function) { }

    R run(typename CallbackParamTraits<A1>::ForwardType a1) {
        return _function(CallbackForward(a1));
    }
    
private:
    R (*_function)(A1);
};

template <typename R, typename T, typename A1>
class RunnableAdapter<R(T::*)(A1)> {
public:
    typedef R (RunType)(T*, A1);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1)) : _method(method) { }

    R run(T* object, typename CallbackParamTraits<A1>::ForwardType a1) {
        return (object->*_method)(CallbackForward(a1));
    }
    
private:
    R (T::*_method)(A1);
};

template <typename R, typename T, typename A1>
class RunnableAdapter<R(T::*)(A1) const> {
public:
    typedef R (RunType)(const T*, A1);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1) const) : _method(method) { }

    R run(const T* object, typename CallbackParamTraits<A1>::ForwardType a1) {
        return (object->*_method)(CallbackForward(a1));
    }
    
private:
    R (T::*_method)(A1) const;
};

template <typename R, typename A1, typename A2>
class RunnableAdapter<R(*)(A1, A2)> {
public:
    typedef R (RunType)(A1, A2);
    
    explicit RunnableAdapter(R(*function)(A1, A2)) : _function(function) { }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2) {
        return _function(CallbackForward(a1), CallbackForward(a2));
    }
    
private:
    R (*_function)(A1, A2);
};

template <typename R, typename T, typename A1, typename A2>
class RunnableAdapter<R(T::*)(A1, A2)> {
public:
    typedef R (RunType)(T*, A1, A2);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2)) : _method(method) { }

    R run(T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2));
    }
    
private:
    R (T::*_method)(A1, A2);
};

template <typename R, typename T, typename A1, typename A2>
class RunnableAdapter<R(T::*)(A1, A2) const> {
public:
    typedef R (RunType)(const T*, A1, A2);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2) const) : _method(method) { }

    R run(const T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2));
    }
    
private:
    R (T::*_method)(A1, A2) const;
};

template <typename R, typename A1, typename A2, typename A3>
class RunnableAdapter<R(*)(A1, A2, A3)> {
public:
    typedef R (RunType)(A1, A2, A3);
    
    explicit RunnableAdapter(R(*function)(A1, A2, A3)) : _function(function) { }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3) {
        return _function(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3));
    }
    
private:
    R (*_function)(A1, A2, A3);
};

template <typename R, typename T, typename A1, typename A2, typename A3>
class RunnableAdapter<R(T::*)(A1, A2, A3)> {
public:
    typedef R (RunType)(T*, A1, A2, A3);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3)) : _method(method) { }

    R run(T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3));
    }
    
private:
    R (T::*_method)(A1, A2, A3);
};

template <typename R, typename T, typename A1, typename A2, typename A3>
class RunnableAdapter<R(T::*)(A1, A2, A3) const> {
public:
    typedef R (RunType)(const T*, A1, A2, A3);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3) const) : _method(method) { }

    R run(const T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3));
    }
    
private:
    R (T::*_method)(A1, A2, A3) const;
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
class RunnableAdapter<R(*)(A1, A2, A3, A4)> {
public:
    typedef R (RunType)(A1, A2, A3, A4);
    
    explicit RunnableAdapter(R(*function)(A1, A2, A3, A4)) : _function(function) { }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4) {
        return _function(CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4));
    }
    
private:
    R (*_function)(A1, A2, A3, A4);
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4)> {
public:
    typedef R (RunType)(T*, A1, A2, A3, A4);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4)) : _method(method) { }

    R run(T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4) {
        return (object->*_method)(CallbackForward(a1),
            CallbackForward(a2), CallbackForward(a3), CallbackForward(a4));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4);
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4) const> {
public:
    typedef R (RunType)(const T*, A1, A2, A3, A4);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4) const) : _method(method) { }

    R run(const T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4) {
        return (object->*_method)(CallbackForward(a1),
            CallbackForward(a2), CallbackForward(a3), CallbackForward(a4));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4) const;
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
class RunnableAdapter<R(*)(A1, A2, A3, A4, A5)> {
public:
    typedef R (RunType)(A1, A2, A3, A4, A5);
    
    explicit RunnableAdapter(R(*function)(A1, A2, A3, A4, A5)) : _function(function) { }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5) {
        return _function(CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4), CallbackForward(a5));
    }
    
private:
    R (*_function)(A1, A2, A3, A4, A5);
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4,
    typename A5>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4, A5)> {
public:
    typedef R (RunType)(T*, A1, A2, A3, A4, A5);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4, A5)) : _method(method) { }

    R run(T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4), CallbackForward(a5));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4, A5);
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4,
    typename A5>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4, A5) const> {
public:
    typedef R (RunType)(const T*, A1, A2, A3, A4, A5);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4, A5) const) : _method(method) { }

    R run(const T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4), CallbackForward(a5));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4, A5) const;
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6>
class RunnableAdapter<R(*)(A1, A2, A3, A4, A5, A6)> {
public:
    typedef R (RunType)(A1, A2, A3, A4, A5, A6);
    
    explicit RunnableAdapter(R(*function)(A1, A2, A3, A4, A5, A6)) : _function(function) { }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6) {
        return _function(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6));
    }
    
private:
    R (*_function)(A1, A2, A3, A4, A5, A6);
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4, A5, A6)> {
public:
    typedef R (RunType)(T*, A1, A2, A3, A4, A5, A6);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4, A5, A6)) : _method(method) { }

    R run(T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4, A5, A6);
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4, A5, A6) const> {
public:
    typedef R (RunType)(const T*, A1, A2, A3, A4, A5, A6);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4, A5, A6) const) : _method(method) { }

    R run(const T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4, A5, A6) const;
};

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6, typename A7>
class RunnableAdapter<R(*)(A1, A2, A3, A4, A5, A6, A7)> {
public:
    typedef R (RunType)(A1, A2, A3, A4, A5, A6, A7);
    
    explicit RunnableAdapter(R(*function)(A1, A2, A3, A4, A5, A6, A7)) : _function(function) { }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6,
        typename CallbackParamTraits<A7>::ForwardType a7) {
        return _function(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6), CallbackForward(a7));
    }
    
private:
    R (*_function)(A1, A2, A3, A4, A5, A6, A7);
};

template <typename R, typename T, typename A1, typename A2, typename A3,
    typename A4, typename A5, typename A6, typename A7>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4, A5, A6, A7)> {
public:
    typedef R (RunType)(T*, A1, A2, A3, A4, A5, A6, A7);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4, A5, A6, A7)) : _method(method) { }

    R run(T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6,
        typename CallbackParamTraits<A7>::ForwardType a7) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6), CallbackForward(a7));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4, A5, A6, A7);
};

template <typename R, typename T, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
class RunnableAdapter<R(T::*)(A1, A2, A3, A4, A5, A6, A7) const> {
public:
    typedef R (RunType)(const T*, A1, A2, A3, A4, A5, A6, A7);
    typedef TrueType IsMethod;
    
    explicit RunnableAdapter(R(T::*method)(A1, A2, A3, A4, A5, A6, A7) const) : _method(method) { }

    R run(const T* object, typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6,
        typename CallbackParamTraits<A7>::ForwardType a7) {
        return (object->*_method)(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6), CallbackForward(a7));
    }
    
private:
    R (T::*_method)(A1, A2, A3, A4, A5, A6, A7) const;
};

} /* namespace internal */
} /* namespace closure */

#endif /* __CLOSURE_RUNNABLE_ADAPTER_H__ */

