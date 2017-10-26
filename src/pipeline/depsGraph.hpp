/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (depsGraph.hpp)
* Copyright (C) 2017 Ryan Schroeder, Colin Unger
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
*************************************************************************/

#pragma once

#include <cassert>
#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "dataGraph.hpp"

namespace fps {
class FDepsGraph;
class FDepsGraphEdge;

class FDepsGraphNode : public fun::FObject {
  std::string                                m_name;
  std::vector<fun::FWeakPtr<FDepsGraphEdge>> m_ins, m_outs;
  fun::FWeakPtr<FDataGraphNodeBase>          m_data;
  bool                                       m_ready = false;

  void statSelf();

  void stat();

public:
  FDepsGraphNode(const std::string &);

  inline const fun::FPtr<FDepsGraphEdge> &operator<<(
      const fun::FPtr<FDepsGraphEdge> &);

  inline const fun::FPtr<FDepsGraphEdge> &operator>>(
      const fun::FPtr<FDepsGraphEdge> &);

  template <typename T>
  fun::FPtr<FDataGraphNode<T>> data(T value) {
    auto node = fnew<FDataGraphNode<T>>(value);
    m_data    = fun::weak(node);
    return node;
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

  inline void in(fun::FPtr<FDepsGraphNode> in) {
    m_ins.push_back(fun::weak(in));
    in->m_outs.push_back(fun::weak(this));
  }

  inline void out(fun::FPtr<FDepsGraphNode> out) {
    m_outs.push_back(fun::weak(out));
    out->m_ins.push_back(fun::weak(this));
  }

  inline const fun::FPtr<FDepsGraphNode> &operator<<(
      const fun::FPtr<FDepsGraphNode> &node) {
    in(node);
    return node;
  }

  inline const fun::FPtr<FDepsGraphNode> &operator>>(
      const fun::FPtr<FDepsGraphNode> &node) {
    out(node);
    return node;
  }

  friend class FDepsGraphNode;
  friend class FDepsGraph;
};

inline const fun::FPtr<FDepsGraphEdge> &FDepsGraphNode::operator<<(
    const fun::FPtr<FDepsGraphEdge> &edge) {
  edge->out(fun::wrap(this));
  return edge;
}

inline const fun::FPtr<FDepsGraphEdge> &FDepsGraphNode::operator>>(
    const fun::FPtr<FDepsGraphEdge> &edge) {
  edge->in(fun::wrap(this));
  return edge;
}

class FDepsGraph : public fun::FObject {
  std::vector<fun::FPtr<FDepsGraphNode>>    m_nodes;
  std::vector<fun::FPtr<FDepsGraphEdge>>    m_edges;
  std::queue<fun::FWeakPtr<FDepsGraphEdge>> m_q;
  bool                                      m_run = false;

public:
  fun::FPtr<FDepsGraphNode> node(const std::string &);

  fun::FPtr<FDepsGraphEdge> edge(const std::string &,
                                 fun::FPtr<FDataGraphRuleBase>);

  void run();

  void dot(std::ostream &);

  friend class FDepsGraphEdge;
};
}
