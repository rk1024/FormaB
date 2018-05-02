/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (depsGraph.hpp)
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

#include <cassert>
#include <queue>
#include <unordered_set>
#include <vector>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "diagnostic/logger.hpp"

#include "depsGraph/data.hpp"

// NB: helpers.hpp is included at the end of this file

namespace fpp {
class FDepsGraph;
class FDepsGraphEdge;

template <typename>
class FDepsNodeHelper;

template <typename, typename, typename...>
class FDepsEdgeHelper;

class FDepsGraphNode : public fun::FObject {
  std::string                                m_name;
  std::vector<fun::FWeakPtr<FDepsGraphEdge>> m_ins, m_outs;
  fun::FPtr<FDataGraphNodeBase>              m_data;
  bool                                       m_ready = false;

  void statSelf();

  void stat();

public:
  auto data() const { return m_data; }

  FDepsGraphNode(const std::string &);

  template <typename T>
  fun::FPtr<FDataGraphNode<T>> data(const T &value) {
    assert(m_data.nil());
    m_data = fnew<FDataGraphNode<T>>(value);
    return m_data;
  }

  template <typename T>
  fun::FPtr<FDataGraphNode<T>> data() {
    assert(m_data.nil());
    m_data = fnew<FDataGraphNode<T>>();
    return m_data;
  }

  friend class FDepsGraphEdge;
  friend class FDepsGraph;
};

class FDepsGraphEdge : public fun::FObject {
  std::string                                m_name;
  fun::FWeakPtr<FDepsGraph>                  m_graph;
  fun::FPtr<FDataGraphRuleBase>              m_rule;
  std::vector<fun::FWeakPtr<FDepsGraphNode>> m_ins, m_outs;
  enum { Closed, Open, Done } m_state = Closed;

  void stat();

  void run();

public:
  FDepsGraphEdge(const std::string &,
                 fun::FWeakPtr<FDepsGraph>,
                 fun::FPtr<FDataGraphRuleBase>);

  void in(fun::FPtr<FDepsGraphNode> in) {
    m_ins.push_back(fun::weak(in));
    in->m_outs.push_back(fun::weak(this));
  }

  void out(fun::FPtr<FDepsGraphNode> out) {
    m_outs.push_back(fun::weak(out));
    out->m_ins.push_back(fun::weak(this));
  }

  friend class FDepsGraphNode;
  friend class FDepsGraph;
};

class FDepsGraph : public fun::FObject {
  std::vector<fun::FPtr<FDepsGraphNode>>    m_nodes;
  std::vector<fun::FPtr<FDepsGraphEdge>>    m_edges;
  std::queue<fun::FWeakPtr<FDepsGraphEdge>> m_q;
  bool                                      m_run = false;

public:
  template <typename T>
  FDepsNodeHelper<T> node(const std::string &, const T &);

  template <typename T>
  FDepsNodeHelper<T> node(const std::string &);

  template <typename T, typename TOut, typename... TArgs>
  FDepsEdgeHelper<T, TOut, TArgs...> edge(
      const std::string &, const fun::FMFPtr<T, TOut, TArgs...> &);

  template <typename T, typename TOut, typename U, typename... TArgs>
  FDepsEdgeHelper<T, TOut, TArgs...> edge(const std::string &,
                                          fun::FPtr<T>,
                                          TOut (U::*)(TArgs...));

  void run(const fdi::FLogger &);

  void dot(std::ostream &);

  friend class FDepsGraphEdge;
};
} // namespace fpp

#include "depsGraph/helpers.hpp"

namespace fpp {
template <typename T>
FDepsNodeHelper<T> FDepsGraph::node(const std::string &name, const T &value) {
  auto &node = m_nodes.emplace_back(fnew<FDepsGraphNode>(name));
  return FDepsNodeHelper<T>(node, node->data<T>(value));
}

template <typename T>
FDepsNodeHelper<T> FDepsGraph::node(const std::string &name) {
  auto &node = m_nodes.emplace_back(fnew<FDepsGraphNode>(name));
  return FDepsNodeHelper<T>(node, node->data<T>());
}

template <typename T, typename TOut, typename... TArgs>
FDepsEdgeHelper<T, TOut, TArgs...> FDepsGraph::edge(
    const std::string &name, const fun::FMFPtr<T, TOut, TArgs...> &ptr) {
  auto rule = fnew<FDataGraphRule<T, TOut, TArgs...>>(ptr);

  auto &edge = m_edges.emplace_back(fnew<FDepsGraphEdge>(name));

  return FDepsEdgeHelper<T, TOut, TArgs...>(edge, rule);
}

template <typename T, typename TOut, typename U, typename... TArgs>
FDepsEdgeHelper<T, TOut, TArgs...> FDepsGraph::edge(const std::string &name,
                                                    fun::FPtr<T>       ptr,
                                                    TOut (U::*pmf)(TArgs...)) {
  auto rule = fnew<FDataGraphRule<T, TOut, TArgs...>>(ptr->*pmf);

  auto &edge = m_edges.emplace_back(
      fnew<FDepsGraphEdge>(name, fun::weak(this), rule));

  return FDepsEdgeHelper<T, TOut, TArgs...>(edge, rule);
}
} // namespace fpp
