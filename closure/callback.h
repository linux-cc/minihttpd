#ifndef __CLOSURE_CALLBACK_H__
#define __CLOSURE_CALLBACK_H__

#include "closure/bind_state.h"

namespace closure {
namespace internal {

class CallbackBase {
public:
    bool isNull() const {
        return _bindState.get() == NULL;
    }
    
    void reset() {
        _polymorphicInvoke = NULL;
        _bindState = NULL;
    }
    
protected:
    explicit CallbackBase(BindStateBase* bindState)
        :_bindState(bindState), _polymorphicInvoke(NULL) {}
    
    ~CallbackBase() { }
    
    bool equals(const CallbackBase& other) const {
        return _bindState.get() == other._bindState.get() 
            && _polymorphicInvoke == other._polymorphicInvoke;
    }
    
    ScopedRefptr<BindStateBase> _bindState;
    typedef void(*InvokeFuncStorage)(void);
    InvokeFuncStorage _polymorphicInvoke;
};

} /* namespace internal */

using internal::BindState;
using internal::BindStateBase;
using internal::CallbackParamTraits;
using internal::CallbackForward;

template <typename Functor>
class Callback;

template <typename R>
class Callback<R(void)> : public internal::CallbackBase {
public:
    typedef R(RunType)();

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run() const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);

        return f(_bindState.get());
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*);
};

template <typename R, typename A1>
class Callback<R(A1)> : public internal::CallbackBase {
public:
    typedef R(RunType)(A1);

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run(typename CallbackParamTraits<A1>::ForwardType a1) const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);
        return f(_bindState.get(), CallbackForward(a1));
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*, typename CallbackParamTraits<A1>::ForwardType);
};

template <typename R, typename A1, typename A2>
class Callback<R(A1, A2)> : public internal::CallbackBase {
public:
    typedef R(RunType)(A1, A2);

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2) const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);
        return f(_bindState.get(), CallbackForward(a1), CallbackForward(a2));
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*,
        typename CallbackParamTraits<A1>::ForwardType,
        typename CallbackParamTraits<A2>::ForwardType);
};

template <typename R, typename A1, typename A2, typename A3>
class Callback<R(A1, A2, A3)> : public internal::CallbackBase {
public:
    typedef R(RunType)(A1, A2, A3);

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3) const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);
        return f(_bindState.get(), CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3));
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*,
        typename CallbackParamTraits<A1>::ForwardType,
        typename CallbackParamTraits<A2>::ForwardType,
        typename CallbackParamTraits<A3>::ForwardType);
};

template <typename R, typename A1, typename A2, typename A3, typename A4>
class Callback<R(A1, A2, A3, A4)> : public internal::CallbackBase {
public:
    typedef R(RunType)(A1, A2, A3, A4);

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4) const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);
        return f(_bindState.get(), CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4));
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*,
        typename CallbackParamTraits<A1>::ForwardType,
        typename CallbackParamTraits<A2>::ForwardType,
        typename CallbackParamTraits<A3>::ForwardType,
        typename CallbackParamTraits<A4>::ForwardType);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5>
class Callback<R(A1, A2, A3, A4, A5)> : public internal::CallbackBase {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5);

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5) const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);
        return f(_bindState.get(), CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4), CallbackForward(a5));
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*,
        typename CallbackParamTraits<A1>::ForwardType,
        typename CallbackParamTraits<A2>::ForwardType,
        typename CallbackParamTraits<A3>::ForwardType,
        typename CallbackParamTraits<A4>::ForwardType,
        typename CallbackParamTraits<A5>::ForwardType);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6>
class Callback<R(A1, A2, A3, A4, A5, A6)> : public internal::CallbackBase {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5, A6);

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6) const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);
        return f(_bindState.get(), CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4), CallbackForward(a5),
            CallbackForward(a6));
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*,
        typename CallbackParamTraits<A1>::ForwardType,
        typename CallbackParamTraits<A2>::ForwardType,
        typename CallbackParamTraits<A3>::ForwardType,
        typename CallbackParamTraits<A4>::ForwardType,
        typename CallbackParamTraits<A5>::ForwardType,
        typename CallbackParamTraits<A6>::ForwardType);
};

template <typename R, typename A1, typename A2, typename A3, typename A4,
    typename A5, typename A6, typename A7>
class Callback<R(A1, A2, A3, A4, A5, A6, A7)> : public internal::CallbackBase {
public:
    typedef R(RunType)(A1, A2, A3, A4, A5, A6, A7);

    Callback() : CallbackBase(NULL) { }

    template <typename Runnable, typename BindRunType, typename BoundArgsType>
    Callback(BindState<Runnable, BindRunType, BoundArgsType>* bindState)
        : CallbackBase(bindState) {
        PolymorphicInvoke invokeFunc = &BindState<Runnable, BindRunType, BoundArgsType>
            ::InvokerType::run;
        _polymorphicInvoke = reinterpret_cast<InvokeFuncStorage>(invokeFunc);
    }

    bool equals(const Callback& other) const {
        return CallbackBase::equals(other);
    }

    R run(typename CallbackParamTraits<A1>::ForwardType a1,
        typename CallbackParamTraits<A2>::ForwardType a2,
        typename CallbackParamTraits<A3>::ForwardType a3,
        typename CallbackParamTraits<A4>::ForwardType a4,
        typename CallbackParamTraits<A5>::ForwardType a5,
        typename CallbackParamTraits<A6>::ForwardType a6,
        typename CallbackParamTraits<A7>::ForwardType a7) const {
        PolymorphicInvoke f = reinterpret_cast<PolymorphicInvoke>(_polymorphicInvoke);
        return f(_bindState.get(), CallbackForward(a1), CallbackForward(a2),
            CallbackForward(a3), CallbackForward(a4), CallbackForward(a5),
            CallbackForward(a6), CallbackForward(a7));
    }

private:
    typedef R(*PolymorphicInvoke)(BindStateBase*,
        typename CallbackParamTraits<A1>::ForwardType,
        typename CallbackParamTraits<A2>::ForwardType,
        typename CallbackParamTraits<A3>::ForwardType,
        typename CallbackParamTraits<A4>::ForwardType,
        typename CallbackParamTraits<A5>::ForwardType,
        typename CallbackParamTraits<A6>::ForwardType,
        typename CallbackParamTraits<A7>::ForwardType);
};

} /* namespace closure */

#endif /* __CLOSURE_CALLBACK_H__ */

