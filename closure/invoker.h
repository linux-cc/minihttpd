#ifndef __CLOSURE_INVOKER_H__
#define __CLOSURE_INVOKER_H__

#include "closure/invoker_helper.h"
#include "closure/callback_param_traits.h"

namespace closure {
namespace internal {

class BindStateBase;

template <int NumBound, typename BindState, typename RunType>
struct Invoker;

template <typename BindState, typename R>
struct Invoker<0, BindState, R()> {
    typedef R(UnboundRunType)();

    static R run(BindStateBase* base) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void()>::invoke(state->_runnable);
    }
};

template <typename BindState, typename R, typename X1>
struct Invoker<0, BindState, R(X1)> {
    typedef R(UnboundRunType)(X1);
    
    static R run(BindStateBase* base, typename CallbackParamTraits<X1>::ForwardType x1) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename CallbackParamTraits<X1>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1));
    }
};

template <typename BindState, typename R, typename X1>
struct Invoker<1, BindState, R(X1)> {
    typedef R(UnboundRunType)();
    
    static R run(BindStateBase* base) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1));
    }
};

template <typename BindState, typename R, typename X1, typename X2>
struct Invoker<0, BindState, R(X1, X2)> {
    typedef R(UnboundRunType)(X1, X2);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X1>::ForwardType x1,
        typename CallbackParamTraits<X2>::ForwardType x2) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename CallbackParamTraits<X1>::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2));
    }
};

template <typename BindState, typename R, typename X1, typename X2>
struct Invoker<1, BindState, R(X1, X2)> {
    typedef R(UnboundRunType)(X2);
    
    static R run(BindStateBase* base, typename CallbackParamTraits<X2>::ForwardType x2) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);

        return InvokerHelper<BindState::IsWeakCall::value, R,
            typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2));
    }
};

template <typename BindState, typename R, typename X1, typename X2>
struct Invoker<2, BindState, R(X1, X2)> {
    typedef R(UnboundRunType)();
    
    static R run(BindStateBase* base) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType, typename Bound2UnwrapTraits::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3>
struct Invoker<0, BindState, R(X1, X2, X3)> {
    typedef R(UnboundRunType)(X1, X2, X3);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X1>::ForwardType x1,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename CallbackParamTraits<X1>::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3>
struct Invoker<1, BindState, R(X1, X2, X3)> {
    typedef R(UnboundRunType)(X2, X3);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3>
struct Invoker<2, BindState, R(X1, X2, X3)> {
    typedef R(UnboundRunType)(X3);
    
    static R run(BindStateBase* base, typename CallbackParamTraits<X3>::ForwardType x3) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3>
struct Invoker<3, BindState, R(X1, X2, X3)> {
    typedef R(UnboundRunType)();
    
    static R run(BindStateBase* base) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4>
struct Invoker<0, BindState, R(X1, X2, X3, X4)> {
    typedef R(UnboundRunType)(X1, X2, X3, X4);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X1>::ForwardType x1,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename CallbackParamTraits<X1>::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4>
struct Invoker<1, BindState, R(X1, X2, X3, X4)> {
    typedef R(UnboundRunType)(X2, X3, X4);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4>
struct Invoker<2, BindState, R(X1, X2, X3, X4)> {
    typedef R(UnboundRunType)(X3, X4);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4>
struct Invoker<3, BindState, R(X1, X2, X3, X4)> {
    typedef R(UnboundRunType)(X4);

    static R run(BindStateBase* base, typename CallbackParamTraits<X4>::ForwardType x4) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4>
struct Invoker<4, BindState, R(X1, X2, X3, X4)> {
    typedef R(UnboundRunType)();

    static R run(BindStateBase* base) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5>
struct Invoker<0, BindState, R(X1, X2, X3, X4, X5)> {
    typedef R(UnboundRunType)(X1, X2, X3, X4, X5);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X1>::ForwardType x1,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename CallbackParamTraits<X1>::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5>
struct Invoker<1, BindState, R(X1, X2, X3, X4, X5)> {
    typedef R(UnboundRunType)(X2, X3, X4, X5);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5>
struct Invoker<2, BindState, R(X1, X2, X3, X4, X5)> {
    typedef R(UnboundRunType)(X3, X4, X5);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5>
struct Invoker<3, BindState, R(X1, X2, X3, X4, X5)> {
    typedef R(UnboundRunType)(X4, X5);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5>
struct Invoker<4, BindState, R(X1, X2, X3, X4, X5)> {
    typedef R(UnboundRunType)(X5);
    
    static R run(BindStateBase* base, typename CallbackParamTraits<X5>::ForwardType x5) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5>
struct Invoker<5, BindState, R(X1, X2, X3, X4, X5)> {
    typedef R(UnboundRunType)();
    
    static R run(BindStateBase* base) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);
        typename Bound5UnwrapTraits::ForwardType x5 = Bound5UnwrapTraits::unwrap(state->_p5);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename Bound5UnwrapTraits::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6>
struct Invoker<0, BindState, R(X1, X2, X3, X4, X5, X6)> {
    typedef R(UnboundRunType)(X1, X2, X3, X4, X5, X6);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X1>::ForwardType x1,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename CallbackParamTraits<X1>::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5), CallbackForward(x6));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6>
struct Invoker<1, BindState, R(X1, X2, X3, X4, X5, X6)> {
    typedef R(UnboundRunType)(X2, X3, X4, X5, X6);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5), CallbackForward(x6));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6>
struct Invoker<2, BindState, R(X1, X2, X3, X4, X5, X6)> {
    typedef R(UnboundRunType)(X3, X4, X5, X6);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5), CallbackForward(x6));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6>
struct Invoker<3, BindState, R(X1, X2, X3, X4, X5, X6)> {
    typedef R(UnboundRunType)(X4, X5, X6);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5), CallbackForward(x6));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6>
struct Invoker<4, BindState, R(X1, X2, X3, X4, X5, X6)> {
    typedef R(UnboundRunType)(X5, X6);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5), CallbackForward(x6));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6>
struct Invoker<5, BindState, R(X1, X2, X3, X4, X5, X6)> {
    typedef R(UnboundRunType)(X6);
    
    static R run(BindStateBase* base, typename CallbackParamTraits<X6>::ForwardType x6) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);
        typename Bound5UnwrapTraits::ForwardType x5 = Bound5UnwrapTraits::unwrap(state->_p5);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename Bound5UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5), CallbackForward(x6));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6>
struct Invoker<6, BindState, R(X1, X2, X3, X4, X5, X6)> {
    typedef R(UnboundRunType)();
    
    static R run(BindStateBase* base) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);
        typename Bound5UnwrapTraits::ForwardType x5 = Bound5UnwrapTraits::unwrap(state->_p5);
        typename Bound6UnwrapTraits::ForwardType x6 = Bound6UnwrapTraits::unwrap(state->_p6);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename Bound5UnwrapTraits::ForwardType,
                typename Bound6UnwrapTraits::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2),
                CallbackForward(x3), CallbackForward(x4), CallbackForward(x5), CallbackForward(x6));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<0, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)(X1, X2, X3, X4, X5, X6, X7);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X1>::ForwardType x1,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6,
        typename CallbackParamTraits<X7>::ForwardType x7) {
        BindState* state = static_cast<BindState*>(base);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename CallbackParamTraits<X1>::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType,
                typename CallbackParamTraits<X7>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<1, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)(X2, X3, X4, X5, X6, X7);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X2>::ForwardType x2,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6,
        typename CallbackParamTraits<X7>::ForwardType x7) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X2>::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType,
                typename CallbackParamTraits<X7>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<2, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)(X3, X4, X5, X6, X7);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X3>::ForwardType x3,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6,
        typename CallbackParamTraits<X7>::ForwardType x7) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X3>::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType,
                typename CallbackParamTraits<X7>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<3, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)(X4, X5, X6, X7);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X4>::ForwardType x4,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6,
        typename CallbackParamTraits<X7>::ForwardType x7) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X4>::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType,
                typename CallbackParamTraits<X7>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<4, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)(X5, X6, X7);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X5>::ForwardType x5,
        typename CallbackParamTraits<X6>::ForwardType x6,
        typename CallbackParamTraits<X7>::ForwardType x7) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X5>::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType,
                typename CallbackParamTraits<X7>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<5, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)(X6, X7);

    static R run(BindStateBase* base,
        typename CallbackParamTraits<X6>::ForwardType x6,
        typename CallbackParamTraits<X7>::ForwardType x7) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;

        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);
        typename Bound5UnwrapTraits::ForwardType x5 = Bound5UnwrapTraits::unwrap(state->_p5);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename Bound5UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X6>::ForwardType,
                typename CallbackParamTraits<X7>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<6, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)(X7);
    
    static R run(BindStateBase* base, typename CallbackParamTraits<X7>::ForwardType x7) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);
        typename Bound5UnwrapTraits::ForwardType x5 = Bound5UnwrapTraits::unwrap(state->_p5);
        typename Bound6UnwrapTraits::ForwardType x6 = Bound6UnwrapTraits::unwrap(state->_p6);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename Bound5UnwrapTraits::ForwardType,
                typename Bound6UnwrapTraits::ForwardType,
                typename CallbackParamTraits<X7>::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

template <typename BindState, typename R, typename X1, typename X2, typename X3, typename X4,
    typename X5, typename X6, typename X7>
struct Invoker<7, BindState, R(X1, X2, X3, X4, X5, X6, X7)> {
    typedef R(UnboundRunType)();
    
    static R run(BindStateBase* base) {
        typedef typename BindState::Bound1UnwrapTraits Bound1UnwrapTraits;
        typedef typename BindState::Bound2UnwrapTraits Bound2UnwrapTraits;
        typedef typename BindState::Bound3UnwrapTraits Bound3UnwrapTraits;
        typedef typename BindState::Bound4UnwrapTraits Bound4UnwrapTraits;
        typedef typename BindState::Bound5UnwrapTraits Bound5UnwrapTraits;
        typedef typename BindState::Bound6UnwrapTraits Bound6UnwrapTraits;
        typedef typename BindState::Bound7UnwrapTraits Bound7UnwrapTraits;
        
        BindState* state = static_cast<BindState*>(base);
        typename Bound1UnwrapTraits::ForwardType x1 = Bound1UnwrapTraits::unwrap(state->_p1);
        typename Bound2UnwrapTraits::ForwardType x2 = Bound2UnwrapTraits::unwrap(state->_p2);
        typename Bound3UnwrapTraits::ForwardType x3 = Bound3UnwrapTraits::unwrap(state->_p3);
        typename Bound4UnwrapTraits::ForwardType x4 = Bound4UnwrapTraits::unwrap(state->_p4);
        typename Bound5UnwrapTraits::ForwardType x5 = Bound5UnwrapTraits::unwrap(state->_p5);
        typename Bound6UnwrapTraits::ForwardType x6 = Bound6UnwrapTraits::unwrap(state->_p6);
        typename Bound7UnwrapTraits::ForwardType x7 = Bound7UnwrapTraits::unwrap(state->_p7);

        return InvokerHelper<BindState::IsWeakCall::value, R, typename BindState::RunnableType,
            void(typename Bound1UnwrapTraits::ForwardType,
                typename Bound2UnwrapTraits::ForwardType,
                typename Bound3UnwrapTraits::ForwardType,
                typename Bound4UnwrapTraits::ForwardType,
                typename Bound5UnwrapTraits::ForwardType,
                typename Bound6UnwrapTraits::ForwardType,
                typename Bound7UnwrapTraits::ForwardType)>
            ::invoke(state->_runnable, CallbackForward(x1), CallbackForward(x2), CallbackForward(x3),
                CallbackForward(x4), CallbackForward(x5), CallbackForward(x6), CallbackForward(x7));
    }
};

} /* namespace internal */
} /* namespace closure */

#endif /* __CLOSURE_INVOKER_H__ */

