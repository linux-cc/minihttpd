#ifndef __CLOSURE_INVOKER_HELPER_H__
#define __CLOSURE_INVOKER_HELPER_H__

#include "util/util.h"
#include "closure/callback_param_traits.h"

namespace closure {
namespace internal {

template <bool IsWeakCall, typename ReturnType, typename Runnable, typename ArgsType>
struct InvokerHelper;

template <typename ReturnType, typename Runnable>
struct InvokerHelper<false, ReturnType, Runnable, void()>  {
    static ReturnType invoke(Runnable runnable) {
        return runnable.run();
    }
};

template <typename Runnable>
struct InvokerHelper<false, void, Runnable, void()>  {
    static void invoke(Runnable runnable) {
        runnable.run();
    }
};

template <typename ReturnType, typename Runnable, typename A1>
struct InvokerHelper<false, ReturnType, Runnable, void(A1)>  {
    static ReturnType invoke(Runnable runnable, A1 a1) {
        return runnable.run(CallbackForward(a1));
    }
};

template <typename Runnable, typename A1>
struct InvokerHelper<false, void, Runnable, void(A1)>  {
    static void invoke(Runnable runnable, A1 a1) {
        runnable.run(CallbackForward(a1));
    }
};

template <typename Runnable, typename BoundWeakPtr>
struct InvokerHelper<true, void, Runnable, void(BoundWeakPtr)>  {
    static void invoke(Runnable runnable, BoundWeakPtr weakPtr) {
        if (!weakPtr.get()) {
            return;
        }
        runnable.run(weakPtr.get());
    }
};

template <typename ReturnType, typename Runnable, typename A1, typename A2>
struct InvokerHelper<false, ReturnType, Runnable, void(A1, A2)>  {
    static ReturnType invoke(Runnable runnable, A1 a1, A2 a2) {
        return runnable.run(CallbackForward(a1), CallbackForward(a2));
    }
};

template <typename Runnable, typename A1, typename A2>
struct InvokerHelper<false, void, Runnable, void(A1, A2)>  {
    static void invoke(Runnable runnable, A1 a1, A2 a2) {
        runnable.run(CallbackForward(a1), CallbackForward(a2));
    }
};

template <typename Runnable, typename BoundWeakPtr, typename A2>
struct InvokerHelper<true, void, Runnable, void(BoundWeakPtr, A2)>  {
    static void invoke(Runnable runnable, BoundWeakPtr weakPtr, A2 a2) {
        if (!weakPtr.get()) {
            return;
        }
        runnable.run(weakPtr.get(), CallbackForward(a2));
    }
};

template <typename ReturnType, typename Runnable, typename A1, typename A2, typename A3>
struct InvokerHelper<false, ReturnType, Runnable, void(A1, A2, A3)>  {
    static ReturnType invoke(Runnable runnable, A1 a1, A2 a2, A3 a3) {
        return runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3));
    }
};

template <typename Runnable, typename A1, typename A2, typename A3>
struct InvokerHelper<false, void, Runnable, void(A1, A2, A3)>  {
    static void invoke(Runnable runnable, A1 a1, A2 a2, A3 a3) {
        runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3));
    }
};

template <typename Runnable, typename BoundWeakPtr, typename A2, typename A3>
struct InvokerHelper<true, void, Runnable, void(BoundWeakPtr, A2, A3)>  {
    static void invoke(Runnable runnable, BoundWeakPtr weakPtr, A2 a2, A3 a3) {
        if (!weakPtr.get()) {
            return;
        }
        runnable.run(weakPtr.get(), CallbackForward(a2), CallbackForward(a3));
    }
};

template <typename ReturnType, typename Runnable, typename A1, typename A2, typename A3,
    typename A4>
struct InvokerHelper<false, ReturnType, Runnable, void(A1, A2, A3, A4)>  {
    static ReturnType invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4) {
        return runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4));
    }
};

template <typename Runnable, typename A1, typename A2, typename A3, typename A4>
struct InvokerHelper<false, void, Runnable, void(A1, A2, A3, A4)>  {
    static void invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4) {
        runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4));
    }
};

template <typename Runnable, typename BoundWeakPtr, typename A2, typename A3, typename A4>
struct InvokerHelper<true, void, Runnable, void(BoundWeakPtr, A2, A3, A4)>  {
    static void invoke(Runnable runnable, BoundWeakPtr weakPtr, A2 a2, A3 a3, A4 a4) {
        if (!weakPtr.get()) {
            return;
        }
        runnable.run(weakPtr.get(), CallbackForward(a2), CallbackForward(a3), CallbackForward(a4));
    }
};

template <typename ReturnType, typename Runnable, typename A1, typename A2, typename A3,
    typename A4, typename A5>
struct InvokerHelper<false, ReturnType, Runnable, void(A1, A2, A3, A4, A5)>  {
    static ReturnType invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
        return runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5));
    }
};

template <typename Runnable, typename A1, typename A2, typename A3, typename A4, typename A5>
struct InvokerHelper<false, void, Runnable, void(A1, A2, A3, A4, A5)>  {
    static void invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) {
        runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5));
    }
};

template <typename Runnable, typename BoundWeakPtr, typename A2, typename A3, typename A4,
    typename A5>
struct InvokerHelper<true, void, Runnable, void(BoundWeakPtr, A2, A3, A4, A5)>  {
    static void invoke(Runnable runnable, BoundWeakPtr weakPtr, A2 a2, A3 a3, A4 a4, A5 a5) {
        if (!weakPtr.get()) {
            return;
        }
        runnable.run(weakPtr.get(), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5));
    }
};

template <typename ReturnType, typename Runnable, typename A1, typename A2, typename A3,
    typename A4, typename A5, typename A6>
struct InvokerHelper<false, ReturnType, Runnable, void(A1, A2, A3, A4, A5, A6)>  {
    static ReturnType invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
        return runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6));
    }
};

template <typename Runnable, typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6>
struct InvokerHelper<false, void, Runnable, void(A1, A2, A3, A4, A5, A6)>  {
    static void invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
        runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6));
    }
};

template <typename Runnable, typename BoundWeakPtr, typename A2, typename A3, typename A4,
    typename A5, typename A6>
struct InvokerHelper<true, void, Runnable, void(BoundWeakPtr, A2, A3, A4, A5, A6)>  {
    static void invoke(Runnable runnable, BoundWeakPtr weakPtr, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6) {
        if (!weakPtr.get()) {
            return;
        }
        runnable.run(weakPtr.get(), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6));
    }
};

template <typename ReturnType, typename Runnable, typename A1, typename A2, typename A3,
    typename A4, typename A5, typename A6, typename A7>
struct InvokerHelper<false, ReturnType, Runnable, void(A1, A2, A3, A4, A5, A6, A7)>  {
    static ReturnType invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
        return runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6), CallbackForward(a7));
    }
};

template <typename Runnable, typename A1, typename A2, typename A3, typename A4, typename A5,
    typename A6, typename A7>
struct InvokerHelper<false, void, Runnable, void(A1, A2, A3, A4, A5, A6, A7)>  {
    static void invoke(Runnable runnable, A1 a1, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
        runnable.run(CallbackForward(a1), CallbackForward(a2), CallbackForward(a3),
            CallbackForward(a4), CallbackForward(a5), CallbackForward(a6), CallbackForward(a7));
    }
};

template <typename Runnable, typename BoundWeakPtr, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
struct InvokerHelper<true, void, Runnable, void(BoundWeakPtr, A2, A3, A4, A5, A6, A7)>  {
    static void invoke(Runnable runnable, BoundWeakPtr weakPtr, A2 a2, A3 a3, A4 a4, A5 a5, A6 a6, A7 a7) {
        if (!weakPtr.get()) {
            return;
        }
        runnable.run(weakPtr.get(), CallbackForward(a2), CallbackForward(a3), CallbackForward(a4),
            CallbackForward(a5), CallbackForward(a6), CallbackForward(a7));
    }
};

template <typename ReturnType, typename Runnable, typename ArgsType>
struct InvokerHelper<true, ReturnType, Runnable, ArgsType> {
    // WeakCalls are only supported for functions with a void return type.
    // Otherwise, the function result would be undefined if the the WeakPtr<>
    // is invalidated.
    COMPILE_ASSERT(util::IsVoid<ReturnType>::value, weak_ptrs_can_only_bind_to_methods_without_return_values);
};

} /* namespace internal */
} /* namespace closure */

#endif /* __CLOSURE_INVOKER_HELPER_H__ */

