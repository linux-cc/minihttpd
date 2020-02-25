#ifndef __CLOSURE_CALLBACK_PARAM_TRAITS_H__
#define __CLOSURE_CALLBACK_PARAM_TRAITS_H__

#include "util/template_util.h"

namespace closure {
namespace internal {

using util::IsMoveOnlyType;
using util::EnableIf;

template <typename T, bool isMoveOnly = IsMoveOnlyType<T>::value>
struct CallbackParamTraits {
    typedef const T& ForwardType;
    typedef T StorageType;
};

template <typename T>
struct CallbackParamTraits<T&, false> {
    typedef T& ForwardType;
    typedef T StorageType;
};

template <typename T, int n>
struct CallbackParamTraits<T[n], false> {
    typedef const T* ForwardType;
    typedef const T* StorageType;
};

template <typename T>
struct CallbackParamTraits<T[], false> {
    typedef const T* ForwardType;
    typedef const T* StorageType;
};

template <typename T>
struct CallbackParamTraits<T, true> {
    typedef T ForwardType;
    typedef T StorageType;
};

template <typename T>
typename EnableIf<!IsMoveOnlyType<T>::value, T>::type& CallbackForward(T& t) {
    return t;
}

template <typename T>
typename EnableIf<IsMoveOnlyType<T>::value, T>::type CallbackForward(T& t) {
    return t.pass();
}

} /* namespace internal */
} /* namespace closure */

#endif /* __CLOSURE_CALLBACK_PARAM_TRAITS_H__ */

