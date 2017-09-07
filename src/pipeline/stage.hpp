#pragma once

#include "util/object/object.hpp"
#include "util/ptr.hpp"

namespace fps {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++1z-extensions"
template <template <typename> typename TContainer,
          typename T,
          typename TExpect = void,
          bool isConst     = true>
class FProducesBase;
#pragma clang diagnostic pop

template <bool, typename>
struct _const_if;

template <typename T>
struct _const_if<false, T> {
  using type = T;
};

template <typename T>
struct _const_if<true, T> {
  using type = const T;
};

template <bool cond, typename T>
using _const_if_t = typename _const_if<cond, T>::type;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++1z-extensions"
template <template <typename> typename TContainer,
          typename T,
          typename TReturn = void,
          bool isConst     = true>
class FAcceptsBase : public virtual fun::FObject {
protected:
  virtual TReturn accept(TContainer<_const_if_t<isConst, T>>) = 0;

  template <template <typename> typename, typename, typename, bool>
  friend class FProducesBase;
};

class FProducesCore : public virtual fun::FObject {
protected:
  template <typename T,
            typename TReturn,
            bool isConst,
            template <typename> typename TContainer,
            typename U>
  static inline TReturn produce(
      FProducesBase<TContainer, T, TReturn, isConst> *produces, U product) {
    return produces->propagate(product);
  }
};

template <template <typename> typename TContainer,
          typename T,
          typename TExpect,
          bool isConst>
class FProducesBase : public virtual FProducesCore {
  fun::FPtr<FAcceptsBase<TContainer, T, TExpect, true>> m_connected;

protected:
  TExpect propagate(TContainer<_const_if_t<isConst, T>> product) {
    if (!m_connected) throw std::runtime_error("no acceptor connected");
    return m_connected->accept(product);
  }

public:
  void connect(
      fun::FPtr<FAcceptsBase<TContainer, T, TExpect, isConst>> acceptor) {
    if (m_connected) throw std::runtime_error("acceptor already connected");
    m_connected = acceptor;
  }

  friend class FProducesCore;
};
#pragma clang diagnostic pop

template <typename T>
using _ptr = T *;

template <typename T>
using _ref = T &;

template <typename T, typename TReturn = void, bool isConst = true>
using FAccepts = FAcceptsBase<fun::FPtr, T, TReturn, isConst>;

template <typename T, typename TReturn = void, bool isConst = true>
using FAcceptsPtr = FAcceptsBase<_ptr, T, TReturn, isConst>;

template <typename T, typename TReturn = void, bool isConst = true>
using FAcceptsRef = FAcceptsBase<_ref, T, TReturn, isConst>;

template <typename T, typename TExpect = void, bool isConst = true>
using FProduces = FProducesBase<fun::FPtr, T, TExpect, isConst>;

template <typename T, typename TExpect = void, bool isConst = true>
using FProducesPtr = FProducesBase<_ptr, T, TExpect, isConst>;

template <typename T, typename TExpect = void, bool isConst = true>
using FProducesRef = FProducesBase<_ref, T, TExpect, isConst>;
}
