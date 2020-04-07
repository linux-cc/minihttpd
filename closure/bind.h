#ifndef __CLOSURE_BIND_H__
#define __CLOSURE_BIND_H__

#include "closure/functor_traits.h"
#include "closure/bind_state.h"
#include "memory/simple_alloc.h"

namespace closure {

using memory::SimpleAlloc;
using util::IsNonConstReference;
using util::IsArray;

template <typename Functor>
Callback<typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void()>::UnboundRunType>
bind(Functor functor) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::BindState<RunnableType, RunType, void()> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor)));
}

template <typename Functor, typename P1>
Callback<typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void(typename internal::CallbackParamTraits<P1>::StorageType)>
    ::UnboundRunType>
bind(Functor functor, const P1& p1) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::FunctionTraits<RunType> BoundFunctorTraits;

    COMPILE_ASSERT(!(IsNonConstReference<typename BoundFunctorTraits::A1Type>::value),
        do_not_bind_functions_with_nonconst_ref);
    COMPILE_ASSERT(internal::HasIsMethodTag<RunnableType>::value ||
        !internal::NeedsScopedRefptrButGetsRawPtr<P1>::value,
        p1_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::HasIsMethodTag<RunnableType>::value ||
        !IsArray<P1>::value, first_bound_argument_to_method_cannot_be_array);

    typedef internal::BindState<RunnableType, RunType,
        void(typename internal::CallbackParamTraits<P1>::StorageType)> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor), p1));
}

template <typename Functor, typename P1, typename P2>
Callback< typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void(typename internal::CallbackParamTraits<P1>::StorageType,
        typename internal::CallbackParamTraits<P2>::StorageType)>
    ::UnboundRunType>
bind(Functor functor, const P1& p1, const P2& p2) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::FunctionTraits<RunType> BoundFunctorTraits;

    COMPILE_ASSERT(!(IsNonConstReference<typename BoundFunctorTraits::A1Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A2Type>::value),
        do_not_bind_functions_with_nonconst_ref);
    COMPILE_ASSERT(internal::HasIsMethodTag<RunnableType>::value ||
        !internal::NeedsScopedRefptrButGetsRawPtr<P1>::value,
        p1_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::HasIsMethodTag<RunnableType>::value ||
        !IsArray<P1>::value, first_bound_argument_to_method_cannot_be_array);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P2>::value,
        p2_is_refcounted_type_and_needs_scoped_refptr);

    typedef internal::BindState<RunnableType, RunType,
        void(typename internal::CallbackParamTraits<P1>::StorageType,
            typename internal::CallbackParamTraits<P2>::StorageType)> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor), p1, p2));
}

template <typename Functor, typename P1, typename P2, typename P3>
Callback<typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void(typename internal::CallbackParamTraits<P1>::StorageType,
        typename internal::CallbackParamTraits<P2>::StorageType,
        typename internal::CallbackParamTraits<P3>::StorageType)>
    ::UnboundRunType>
bind(Functor functor, const P1& p1, const P2& p2, const P3& p3) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::FunctionTraits<RunType> BoundFunctorTraits;

    COMPILE_ASSERT(!(IsNonConstReference<typename BoundFunctorTraits::A1Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A2Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A3Type>::value),
        do_not_bind_functions_with_nonconst_ref);
    COMPILE_ASSERT(internal::HasIsMethodTag<RunnableType>::value ||
        !internal::NeedsScopedRefptrButGetsRawPtr<P1>::value,
        p1_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::HasIsMethodTag<RunnableType>::value ||
        !IsArray<P1>::value, first_bound_argument_to_method_cannot_be_array);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P2>::value,
        p2_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P3>::value,
        p3_is_refcounted_type_and_needs_scoped_refptr);

    typedef internal::BindState<RunnableType, RunType,
        void(typename internal::CallbackParamTraits<P1>::StorageType,
            typename internal::CallbackParamTraits<P2>::StorageType,
            typename internal::CallbackParamTraits<P3>::StorageType)> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor), p1, p2, p3));
}

template <typename Functor, typename P1, typename P2, typename P3, typename P4>
Callback<typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void(typename internal::CallbackParamTraits<P1>::StorageType,
        typename internal::CallbackParamTraits<P2>::StorageType,
        typename internal::CallbackParamTraits<P3>::StorageType,
        typename internal::CallbackParamTraits<P4>::StorageType)>
    ::UnboundRunType>
bind(Functor functor, const P1& p1, const P2& p2, const P3& p3, const P4& p4) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::FunctionTraits<RunType> BoundFunctorTraits;

    COMPILE_ASSERT(!(IsNonConstReference<typename BoundFunctorTraits::A1Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A2Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A3Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A4Type>::value),
        do_not_bind_functions_with_nonconst_ref);
    COMPILE_ASSERT(internal::HasIsMethodTag<RunnableType>::value ||
        !internal::NeedsScopedRefptrButGetsRawPtr<P1>::value,
        p1_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::HasIsMethodTag<RunnableType>::value ||
        !IsArray<P1>::value, first_bound_argument_to_method_cannot_be_array);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P2>::value,
        p2_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P3>::value,
        p3_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P4>::value,
        p4_is_refcounted_type_and_needs_scoped_refptr);

    typedef internal::BindState<RunnableType, RunType,
        void(typename internal::CallbackParamTraits<P1>::StorageType,
            typename internal::CallbackParamTraits<P2>::StorageType,
            typename internal::CallbackParamTraits<P3>::StorageType,
            typename internal::CallbackParamTraits<P4>::StorageType)> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor), p1, p2, p3, p4));
}

template <typename Functor, typename P1, typename P2, typename P3, typename P4,
    typename P5>
Callback<typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void(typename internal::CallbackParamTraits<P1>::StorageType,
        typename internal::CallbackParamTraits<P2>::StorageType,
        typename internal::CallbackParamTraits<P3>::StorageType,
        typename internal::CallbackParamTraits<P4>::StorageType,
        typename internal::CallbackParamTraits<P5>::StorageType)>
    ::UnboundRunType>
bind(Functor functor, const P1& p1, const P2& p2, const P3& p3, const P4& p4, const P5& p5) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::FunctionTraits<RunType> BoundFunctorTraits;

    COMPILE_ASSERT(!(IsNonConstReference<typename BoundFunctorTraits::A1Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A2Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A3Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A4Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A5Type>::value),
        do_not_bind_functions_with_nonconst_ref);
    COMPILE_ASSERT(internal::HasIsMethodTag<RunnableType>::value ||
        !internal::NeedsScopedRefptrButGetsRawPtr<P1>::value,
        p1_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::HasIsMethodTag<RunnableType>::value ||
        !IsArray<P1>::value, first_bound_argument_to_method_cannot_be_array);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P2>::value,
        p2_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P3>::value,
        p3_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P4>::value,
        p4_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P5>::value,
        p5_is_refcounted_type_and_needs_scoped_refptr);

    typedef internal::BindState<RunnableType, RunType,
        void(typename internal::CallbackParamTraits<P1>::StorageType,
            typename internal::CallbackParamTraits<P2>::StorageType,
            typename internal::CallbackParamTraits<P3>::StorageType,
            typename internal::CallbackParamTraits<P4>::StorageType,
            typename internal::CallbackParamTraits<P5>::StorageType)> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor), p1, p2, p3, p4, p5));
}

template <typename Functor, typename P1, typename P2, typename P3, typename P4,
    typename P5, typename P6>
Callback<typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void(typename internal::CallbackParamTraits<P1>::StorageType,
        typename internal::CallbackParamTraits<P2>::StorageType,
        typename internal::CallbackParamTraits<P3>::StorageType,
        typename internal::CallbackParamTraits<P4>::StorageType,
        typename internal::CallbackParamTraits<P5>::StorageType,
        typename internal::CallbackParamTraits<P6>::StorageType)>
    ::UnboundRunType>
bind(Functor functor, const P1& p1, const P2& p2, const P3& p3, const P4& p4,
    const P5& p5, const P6& p6) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::FunctionTraits<RunType> BoundFunctorTraits;

    COMPILE_ASSERT(!(IsNonConstReference<typename BoundFunctorTraits::A1Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A2Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A3Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A4Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A5Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A6Type>::value),
        do_not_bind_functions_with_nonconst_ref);
    COMPILE_ASSERT(internal::HasIsMethodTag<RunnableType>::value ||
        !internal::NeedsScopedRefptrButGetsRawPtr<P1>::value,
        p1_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::HasIsMethodTag<RunnableType>::value ||
        !IsArray<P1>::value, first_bound_argument_to_method_cannot_be_array);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P2>::value,
        p2_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P3>::value,
        p3_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P4>::value,
        p4_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P5>::value,
        p5_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P6>::value,
        p6_is_refcounted_type_and_needs_scoped_refptr);

    typedef internal::BindState<RunnableType, RunType,
        void(typename internal::CallbackParamTraits<P1>::StorageType,
            typename internal::CallbackParamTraits<P2>::StorageType,
            typename internal::CallbackParamTraits<P3>::StorageType,
            typename internal::CallbackParamTraits<P4>::StorageType,
            typename internal::CallbackParamTraits<P5>::StorageType,
            typename internal::CallbackParamTraits<P6>::StorageType)> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor), p1, p2, p3, p4, p5, p6));
}

template <typename Functor, typename P1, typename P2, typename P3, typename P4,
    typename P5, typename P6, typename P7>
Callback<typename internal::BindState<
    typename internal::FunctorTraits<Functor>::RunnableType,
    typename internal::FunctorTraits<Functor>::RunType,
    void(typename internal::CallbackParamTraits<P1>::StorageType,
        typename internal::CallbackParamTraits<P2>::StorageType,
        typename internal::CallbackParamTraits<P3>::StorageType,
        typename internal::CallbackParamTraits<P4>::StorageType,
        typename internal::CallbackParamTraits<P5>::StorageType,
        typename internal::CallbackParamTraits<P6>::StorageType,
        typename internal::CallbackParamTraits<P7>::StorageType)>
    ::UnboundRunType>
bind(Functor functor, const P1& p1, const P2& p2, const P3& p3, const P4& p4,
    const P5& p5, const P6& p6, const P7& p7) {
    typedef typename internal::FunctorTraits<Functor>::RunnableType RunnableType;
    typedef typename internal::FunctorTraits<Functor>::RunType RunType;
    typedef internal::FunctionTraits<RunType> BoundFunctorTraits;

    COMPILE_ASSERT(!(IsNonConstReference<typename BoundFunctorTraits::A1Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A2Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A3Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A4Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A5Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A6Type>::value ||
        IsNonConstReference<typename BoundFunctorTraits::A7Type>::value),
        do_not_bind_functions_with_nonconst_ref);
    COMPILE_ASSERT(internal::HasIsMethodTag<RunnableType>::value ||
        !internal::NeedsScopedRefptrButGetsRawPtr<P1>::value,
        p1_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::HasIsMethodTag<RunnableType>::value ||
        !IsArray<P1>::value, first_bound_argument_to_method_cannot_be_array);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P2>::value,
        p2_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P3>::value,
        p3_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P4>::value,
        p4_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P5>::value,
        p5_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P6>::value,
        p6_is_refcounted_type_and_needs_scoped_refptr);
    COMPILE_ASSERT(!internal::NeedsScopedRefptrButGetsRawPtr<P7>::value,
        p7_is_refcounted_type_and_needs_scoped_refptr);

    typedef internal::BindState<RunnableType, RunType,
        void(typename internal::CallbackParamTraits<P1>::StorageType,
            typename internal::CallbackParamTraits<P2>::StorageType,
            typename internal::CallbackParamTraits<P3>::StorageType,
            typename internal::CallbackParamTraits<P4>::StorageType,
            typename internal::CallbackParamTraits<P5>::StorageType,
            typename internal::CallbackParamTraits<P6>::StorageType,
            typename internal::CallbackParamTraits<P7>::StorageType)> BindState;

    return Callback<typename BindState::UnboundRunType>(
        SimpleAlloc<BindState>::New(internal::MakeRunnable(functor), p1, p2, p3, p4, p5, p6, p7));
}

} /* namespace base */

#endif /* __CLOSURE_BIND_H__ */

