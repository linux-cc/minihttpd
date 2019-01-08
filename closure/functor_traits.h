#ifndef __CLOSURE_FUNCTOR_TRAITS_H__
#define __CLOSURE_FUNCTOR_TRAITS_H__

#include "closure/runnable_adapter.h"
#include "closure/callback.h"

namespace closure {
namespace internal {

template <typename Functor>
struct FunctionTraits;

template <typename R>
struct FunctionTraits<R()> {
    typedef R ReturnType;
};

template <typename R, typename A1>
struct FunctionTraits<R(A1)> {
    typedef R ReturnType;
    typedef A1 A1Type;
};

template <typename R, typename A1, typename A2>
struct FunctionTraits<R(A1, A2)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
};

template <typename R, typename A1, typename A2, typename A3>
struct FunctionTraits<R(A1, A2, A3)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
struct FunctionTraits<R(A1, A2, A3, A4)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5>
struct FunctionTraits<R(A1, A2, A3, A4, A5)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
struct FunctionTraits<R(A1, A2, A3, A4, A5, A6)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
    typedef A6 A6Type;
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
struct FunctionTraits<R(A1, A2, A3, A4, A5, A6, A7)> {
    typedef R ReturnType;
    typedef A1 A1Type;
    typedef A2 A2Type;
    typedef A3 A3Type;
    typedef A4 A4Type;
    typedef A5 A5Type;
    typedef A6 A6Type;
    typedef A7 A7Type;
};

template <typename T>
struct FunctorTraits {
    typedef RunnableAdapter<T> RunnableType;
    typedef typename RunnableType::RunType RunType;
};

template <typename T>
struct FunctorTraits<Callback<T> > {
    typedef Callback<T> RunnableType;
    typedef typename Callback<T>::RunType RunType;
};

template <typename T>
typename FunctorTraits<T>::RunnableType
MakeRunnable(const T& t) {
    return RunnableAdapter<T>(t);
}

template <typename T>
const typename FunctorTraits<Callback<T> >::RunnableType&
MakeRunnable(const Callback<T>& t) {
    return t;
}

} /* namespace internal */
} /* namespace closure */

#endif /* __CLOSURE_FUNCTOR_TRAITS_H__ */

