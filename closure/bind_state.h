#ifndef __CLOSURE_BIND_STATE_H__
#define __CLOSURE_BIND_STATE_H__

#include "util/scoped_ref.h"
#include "closure/invoker.h"
#include "closure/bind_helper.h"
#include "memory/simple_alloc.h"

namespace closure {
namespace internal {

class BindStateBase : public util::RefCountedThreadSafe<BindStateBase> {
protected:
    friend class memory::SimpleAlloc<BindStateBase>;
    virtual ~BindStateBase() { }
};

template <typename Runnable, typename RunType, typename BoundArgsType>
struct BindState;

template <typename Runnable, typename RunType>
struct BindState<Runnable, RunType, void()> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef FalseType IsWeakCall;
    typedef Invoker<0, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;

    explicit BindState(const Runnable& runnable)
        : _runnable(runnable) { }

    RunnableType _runnable;
};

template <typename Runnable, typename RunType, typename P1>
struct BindState<Runnable, RunType, void(P1)> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef IsWeakMethod<HasIsMethodTag<Runnable>::value, P1> IsWeakCall;
    typedef Invoker<1, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<P1> Bound1UnwrapTraits;

    BindState(const Runnable& runnable, const P1& p1)
        : _runnable(runnable), _p1(p1) {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::incRef(_p1);
    }

    ~BindState() {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::decRef(_p1);
    }

    RunnableType _runnable;
    P1 _p1;
};

template <typename Runnable, typename RunType, typename P1, typename P2>
struct BindState<Runnable, RunType, void(P1, P2)> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef IsWeakMethod<HasIsMethodTag<Runnable>::value, P1> IsWeakCall;
    typedef Invoker<2, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;
    typedef UnwrapTraits<P1> Bound1UnwrapTraits;
    typedef UnwrapTraits<P2> Bound2UnwrapTraits;

    BindState(const Runnable& runnable, const P1& p1, const P2& p2)
        : _runnable(runnable), _p1(p1), _p2(p2) {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::incRef(_p1);
    }

    ~BindState() {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::decRef(_p1);
    }

    RunnableType _runnable;
    P1 _p1;
    P2 _p2;
};

template <typename Runnable, typename RunType, typename P1, typename P2,
    typename P3>
struct BindState<Runnable, RunType, void(P1, P2, P3)> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef IsWeakMethod<HasIsMethodTag<Runnable>::value, P1> IsWeakCall;
    typedef Invoker<3, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;

    typedef UnwrapTraits<P1> Bound1UnwrapTraits;
    typedef UnwrapTraits<P2> Bound2UnwrapTraits;
    typedef UnwrapTraits<P3> Bound3UnwrapTraits;

    BindState(const Runnable& runnable, const P1& p1, const P2& p2, const P3& p3)
        : _runnable(runnable), _p1(p1), _p2(p2), _p3(p3) {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::incRef(_p1);
    }

    ~BindState() {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::decRef(_p1);
    }

    RunnableType _runnable;
    P1 _p1;
    P2 _p2;
    P3 _p3;
};

template <typename Runnable, typename RunType, typename P1, typename P2, typename P3,
    typename P4>
struct BindState<Runnable, RunType, void(P1, P2, P3, P4)> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef IsWeakMethod<HasIsMethodTag<Runnable>::value, P1> IsWeakCall;
    typedef Invoker<4, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;

    typedef UnwrapTraits<P1> Bound1UnwrapTraits;
    typedef UnwrapTraits<P2> Bound2UnwrapTraits;
    typedef UnwrapTraits<P3> Bound3UnwrapTraits;
    typedef UnwrapTraits<P4> Bound4UnwrapTraits;

    BindState(const Runnable& runnable, const P1& p1, const P2& p2, const P3& p3, const P4& p4)
        : _runnable(runnable), _p1(p1), _p2(p2), _p3(p3), _p4(p4) {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::incRef(_p1);
    }

    ~BindState() {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::decRef(_p1);
    }

    RunnableType _runnable;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
};

template <typename Runnable, typename RunType, typename P1, typename P2, typename P3,
    typename P4, typename P5>
struct BindState<Runnable, RunType, void(P1, P2, P3, P4, P5)> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef IsWeakMethod<HasIsMethodTag<Runnable>::value, P1> IsWeakCall;
    typedef Invoker<5, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;

    typedef UnwrapTraits<P1> Bound1UnwrapTraits;
    typedef UnwrapTraits<P2> Bound2UnwrapTraits;
    typedef UnwrapTraits<P3> Bound3UnwrapTraits;
    typedef UnwrapTraits<P4> Bound4UnwrapTraits;
    typedef UnwrapTraits<P5> Bound5UnwrapTraits;

    BindState(const Runnable& runnable, const P1& p1, const P2& p2, const P3& p3,
        const P4& p4, const P5& p5) : _runnable(runnable), _p1(p1), _p2(p2), _p3(p3),
        _p4(p4), _p5(p5) {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::incRef(_p1);
    }

    ~BindState() {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::decRef(_p1);
    }

    RunnableType _runnable;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
    P5 _p5;
};

template <typename Runnable, typename RunType, typename P1, typename P2, typename P3,
    typename P4, typename P5, typename P6>
struct BindState<Runnable, RunType, void(P1, P2, P3, P4, P5, P6)> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef IsWeakMethod<HasIsMethodTag<Runnable>::value, P1> IsWeakCall;
    typedef Invoker<6, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;

    typedef UnwrapTraits<P1> Bound1UnwrapTraits;
    typedef UnwrapTraits<P2> Bound2UnwrapTraits;
    typedef UnwrapTraits<P3> Bound3UnwrapTraits;
    typedef UnwrapTraits<P4> Bound4UnwrapTraits;
    typedef UnwrapTraits<P5> Bound5UnwrapTraits;
    typedef UnwrapTraits<P6> Bound6UnwrapTraits;

    BindState(const Runnable& runnable, const P1& p1, const P2& p2, const P3& p3,
        const P4& p4, const P5& p5, const P6& p6) : _runnable(runnable), _p1(p1),
        _p2(p2), _p3(p3), _p4(p4), _p5(p5), _p6(p6) {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::incRef(_p1);
    }

    ~BindState() {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::decRef(_p1);
    }

    RunnableType _runnable;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
    P5 _p5;
    P6 _p6;
};

template <typename Runnable, typename RunType, typename P1, typename P2, typename P3,
    typename P4, typename P5, typename P6, typename P7>
struct BindState<Runnable, RunType, void(P1, P2, P3, P4, P5, P6, P7)> : public BindStateBase {
    typedef Runnable RunnableType;
    typedef IsWeakMethod<HasIsMethodTag<Runnable>::value, P1> IsWeakCall;
    typedef Invoker<7, BindState, RunType> InvokerType;
    typedef typename InvokerType::UnboundRunType UnboundRunType;

    typedef UnwrapTraits<P1> Bound1UnwrapTraits;
    typedef UnwrapTraits<P2> Bound2UnwrapTraits;
    typedef UnwrapTraits<P3> Bound3UnwrapTraits;
    typedef UnwrapTraits<P4> Bound4UnwrapTraits;
    typedef UnwrapTraits<P5> Bound5UnwrapTraits;
    typedef UnwrapTraits<P6> Bound6UnwrapTraits;
    typedef UnwrapTraits<P7> Bound7UnwrapTraits;

    BindState(const Runnable& runnable, const P1& p1, const P2& p2, const P3& p3,
        const P4& p4, const P5& p5, const P6& p6, const P7& p7) : _runnable(runnable),
        _p1(p1), _p2(p2), _p3(p3), _p4(p4), _p5(p5), _p6(p6), _p7(p7) {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::incRef(_p1);
    }

    ~BindState() {
        MaybeRefcount<HasIsMethodTag<Runnable>::value, P1>::decRef(_p1);
    }

    RunnableType _runnable;
    P1 _p1;
    P2 _p2;
    P3 _p3;
    P4 _p4;
    P5 _p5;
    P6 _p6;
    P7 _p7;
};

} /* namespace internal */
} /* namespace closure */

#endif /* __CLOSURE_BIND_STATE_H__ */

