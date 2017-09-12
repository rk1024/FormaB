#pragma once

#include <vector>

#include "util/autoref.hpp"
#include "util/cons.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"
#include "util/range.hpp"

namespace fps {
template <typename T, typename TOut, typename... TArgs>
class FDataGraphRule;

class FDataGraphNodeBase : public fun::FObject {};

template <typename T>
class FDataGraphNode : public FDataGraphNodeBase {
  T m_data;

public:
  inline fun::autoref<T> data() { return m_data; }
  inline void data(fun::autoref<T> data) { m_data = data; }

  FDataGraphNode(fun::autoref<T> data) : m_data(data) {}

  template <typename U, typename TOut>
  const fun::FPtr<FDataGraphRule<U, TOut, T>> &operator<<(
      const fun::FPtr<FDataGraphRule<U, TOut, T>> &);

  template <typename U, typename TOut>
  const fun::FPtr<FDataGraphRule<U, TOut, T>> &operator>>(
      const fun::FPtr<FDataGraphRule<U, TOut, T>> &);
};

template <>
class FDataGraphNode<void> : public FDataGraphNodeBase {};

class FDataGraphRuleBase : public fun::FObject {
  virtual void run() = 0;

  friend class FDepsGraphEdge;
};

template <typename... TArgs, std::size_t... idcs>
fun::cons_cell<TArgs...> _consNodeExtract_impl(
    fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> nodes,
    fun::idx_range<idcs...>) {
  return fun::cons(nodes.template get<idcs>()->data()...);
}

template <typename... TArgs>
inline fun::cons_cell<TArgs...> _consNodeExtract(
    fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> nodes) {
  return _consNodeExtract_impl(nodes, fun::idx_range_for<TArgs...>());
}

template <typename, typename, typename...>
struct _run_rule;

template <typename T, typename TOut, typename... TArgs>
class FDataGraphRule : public FDataGraphRuleBase {
  fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> m_ins;
  std::vector<fun::FWeakPtr<FDataGraphNode<TOut>>>    m_outs;
  fun::FMFPtr<T, TOut, fun::cons_cell<TArgs...>> m_ptr;

  virtual void run() override { _run_rule<T, TOut, TArgs...>{}(this); }

public:
  FDataGraphRule(fun::FMFPtr<T, TOut, fun::cons_cell<TArgs...>> ptr)
      : m_ptr(ptr) {}

  void in(decltype(m_ins) ins) { m_ins = ins; }

  void out(fun::FWeakPtr<FDataGraphNode<TOut>> out) { m_outs.push_back(out); }

  inline const decltype(m_ins) &operator<<(const decltype(m_ins) &rhs) {
    in(rhs);
    return rhs;
  }

  inline const fun::FPtr<FDataGraphNode<TOut>> &operator>>(
      const fun::FPtr<FDataGraphNode<TOut>> &rhs) {
    out(fun::weak(rhs));
    return rhs;
  }

  friend const fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> &operator>>(
      const fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> &lhs,
      const fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> &rhs);

  friend struct _run_rule<T, TOut, TArgs...>;
};

template <typename T, typename TOut, typename... TArgs>
struct _run_rule {
  inline void operator()(FDataGraphRule<T, TOut, TArgs...> *self) {
    auto ret = self->m_ptr(_consNodeExtract(self->m_ins));
    for (auto out : self->m_outs) out.lock()->data(ret);
  }
};

template <typename T, typename... TArgs>
struct _run_rule<T, void, TArgs...> {
  inline void operator()(FDataGraphRule<T, void, TArgs...> *self) {
    self->m_ptr(_consNodeExtract(self->m_ins));
  }
};

template <typename T, typename TOut, typename... TArgs>
const fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> &operator>>(
    const fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> &lhs,
    const fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> &rhs) {
  rhs->in(lhs);
  return rhs;
}

template <typename T>
template <typename U, typename TOut>
const fun::FPtr<FDataGraphRule<U, TOut, T>> &FDataGraphNode<T>::operator<<(
    const fun::FPtr<FDataGraphRule<U, TOut, T>> &rhs) {
  rhs->out(fun::weak(this));
  return rhs;
}

template <typename T>
template <typename U, typename TOut>
const fun::FPtr<FDataGraphRule<U, TOut, T>> &FDataGraphNode<T>::operator>>(
    const fun::FPtr<FDataGraphRule<U, TOut, T>> &rhs) {
  rhs->in(fun::cons(fun::wrap(this)));
  return rhs;
}

template <typename T, typename TOut, typename... TArgs>
inline fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> rule(
    fun::FMFPtr<T, TOut, fun::cons_cell<TArgs...>> ptr) {
  return fnew<FDataGraphRule<T, TOut, TArgs...>>(ptr);
}

template <typename T, typename TOut, typename U, typename... TArgs>
inline fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> rule(
    fun::FPtr<T> ptr, TOut (U::*pmf)(fun::cons_cell<TArgs...>)) {
  return fnew<FDataGraphRule<U, TOut, TArgs...>>(ptr->*pmf);
}
}
