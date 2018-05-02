/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (helpers.hpp)
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

#include "util/ptr.hpp"

namespace fpp {
class FDepsGraphNode;
class FDepsGraphEdge;

template <typename>
class FDataGraphNode;

template <typename, typename, typename...>
class FDepsEdgeHelper;

template <typename T>
class FDepsNodeHelper {
  fun::FPtr<FDepsGraphNode>    m_node;
  fun::FPtr<FDataGraphNode<T>> m_data;

public:
  FDepsNodeHelper(const fun::FPtr<FDepsGraphNode> &   node,
                  const fun::FPtr<FDataGraphNode<T>> &data) :
      m_node(node),
      m_data(data) {}

  template <typename, typename, typename...>
  friend class FDepsEdgeHelper;
};

template <typename T, typename TOut, typename... TArgs>
class FDepsEdgeHelper {
  fun::FPtr<FDepsGraphEdge>                    m_edge;
  fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> m_rule;

  template <typename U, typename... UArgs>
  void multiIn_impl_deps(const fun::FPtr<U> &car, UArgs &&... cdr) {
    m_edge->in(car);
    return multiIn_impl_deps(std::forward<UArgs>(cdr)...);
  }

  void multiIn_impl_deps() {}

  template <std::size_t... idcs>
  void multiIn_impl(const fun::cons_cell<FDepsNodeHelper<TArgs>...> &ins,
                    std::index_sequence<idcs...>) {
    m_rule->in(fun::cons(ins.template get<idcs>().m_data...));
    multiIn_impl_deps(ins.template get<idcs>().m_node...);
  }

public:
  FDepsEdgeHelper(const fun::FPtr<FDepsGraphEdge> &                   edge,
                  const fun::FPtr<FDataGraphRule<T, TOut, TArgs...>> &rule) :
      m_edge(edge),
      m_rule(rule) {}

  FDepsNodeHelper<TOut> &operator>>(FDepsNodeHelper<TOut> &rhs) {
    m_edge->out(rhs.m_node);
    m_rule->out(fun::weak(rhs.m_data));
    return rhs;
  }

  friend FDepsEdgeHelper &operator>>(
      const fun::cons_cell<FDepsNodeHelper<TArgs>...> &lhs,
      FDepsEdgeHelper &                                rhs) {
    rhs.multiIn_impl(lhs, std::index_sequence_for<TArgs...>());
    return rhs;
  }

  template <typename>
  friend class FDepsNodeHelper;
};
} // namespace fpp
