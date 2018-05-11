/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (data.hpp)
 * Copyright (C) 2017-2018 Ryan Schroeder, Colin Unger
 *
 * FormaB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * FormaB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
 *
 ************************************************************************/

#pragma once

#include <vector>

#include "util/cons.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

namespace fpp {
template <typename T, typename TOut, typename... TArgs>
class FDataGraphRule;

class FDataGraphNodeBase : public fun::FObject {};

template <typename T>
class FDataGraphNode : public FDataGraphNodeBase {
  T m_data;

public:
  constexpr auto &data() { return m_data; }
  constexpr auto &data() const { return m_data; }

  FDataGraphNode(const T &data) : m_data(data) {}
};

template <>
class FDataGraphNode<void> : public FDataGraphNodeBase {};

class FDataGraphRuleBase : public fun::FObject {
  virtual bool run() = 0;

  friend class FDepsGraphEdge;
};

template <typename... TArgs, std::size_t... idcs>
fun::cons_cell<TArgs...> _consNodeExtract_impl(
    const fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> &nodes,
    std::index_sequence<idcs...>) {
  return fun::cons(nodes.template get<idcs>()->data()...);
}

template <typename... TArgs>
inline fun::cons_cell<TArgs...> _consNodeExtract(
    const fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> &nodes) {
  return _consNodeExtract_impl(nodes, std::index_sequence_for<TArgs...>());
}

template <typename T, typename U, typename... TArgs, std::size_t... idcs>
U _consNodeApply_impl(const fun::FMFPtr<T, U, TArgs...> &ptr,
                      const fun::cons_cell<TArgs...> &   args,
                      std::index_sequence<idcs...>) {
  return ptr(args.template get<idcs>()...);
}

template <typename T, typename U, typename... TArgs>
U _consNodeApply(const fun::FMFPtr<T, U, TArgs...> &ptr,
                 const fun::cons_cell<TArgs...> &   args) {
  return _consNodeApply_impl(ptr, args, std::index_sequence_for<TArgs...>());
}

template <typename, typename, typename...>
struct _run_rule;

template <typename T, typename TOut, typename... TArgs>
class FDataGraphRule : public FDataGraphRuleBase {
  fun::cons_cell<fun::FPtr<FDataGraphNode<TArgs>>...> m_ins;
  std::vector<fun::FWeakPtr<FDataGraphNode<TOut>>>    m_outs;
  fun::FMFPtr<T, TOut, TArgs...>                      m_ptr;

  virtual bool run() override {
    try {
      _run_rule<T, TOut, TArgs...>{}(this);
    }
    catch (fdi::logger_raise &) {
      return false;
    }

    return true;
  }

public:
  FDataGraphRule(fun::FMFPtr<T, TOut, TArgs...> ptr) : m_ptr(ptr) {}

  void in(decltype(m_ins) ins) { m_ins = ins; }

  void out(fun::FWeakPtr<FDataGraphNode<TOut>> out) { m_outs.push_back(out); }

  friend struct _run_rule<T, TOut, TArgs...>;
};

template <typename T, typename TOut, typename... TArgs>
struct _run_rule {
  void operator()(FDataGraphRule<T, TOut, TArgs...> *self) {
    auto ret = _consNodeApply(self->m_ptr, _consNodeExtract(self->m_ins));
    for (auto out : self->m_outs) out.lock()->data(ret);
  }
};

template <typename T, typename... TArgs>
struct _run_rule<T, void, TArgs...> {
  void operator()(FDataGraphRule<T, void, TArgs...> *self) {
    _consNodeApply(self->m_ptr, _consNodeExtract(self->m_ins));
  }
};
} // namespace fpp
