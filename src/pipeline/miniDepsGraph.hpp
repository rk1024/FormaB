/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (miniDepsGraph.hpp)
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

#include <ios>
#include <queue>
#include <vector>

#include "util/ptrStore.hpp"

namespace fpp {
template <typename>
class FMiniDepsGraph;

template <typename T>
class FMiniDepsGraphNode {
public:
  using Node  = FMiniDepsGraphNode;
  using Graph = FMiniDepsGraph<T>;

private:
  std::string         m_name;
  T                   m_value;
  Graph *             m_graph;
  std::vector<Node *> m_ins, m_outs;
  bool                m_ready = false;

  void stat() {
    if (m_ready) return;

    for (auto &in : m_ins) {
      if (!in->m_ready) return;
    }

    m_graph->m_q.push(this);

    m_ready = true;
  }

  void statOuts() {
    for (auto out : m_outs) out->stat();
  }

public:
  constexpr auto &value() { return m_value; }
  constexpr auto &value() const { return m_value; }

  FMiniDepsGraphNode(const std::string &name, Graph *graph, const T &value) :
      m_name(name),
      m_value(value),
      m_graph(graph) {}

  friend class FMiniDepsGraph<T>;
};

template <typename T>
class FMiniDepsGraph {
public:
  using Node = FMiniDepsGraphNode<T>;

private:
  fun::FPtrStore<Node> m_nodes;
  std::queue<Node *>   m_q;
  bool                 m_run = false;

public:
  Node *node(const std::string &name, const T &value) {
    return m_nodes.emplace(name, this, value);
  }

  void connect(Node *in, Node *out) {
    in->m_outs.push_back(out);
    out->m_ins.push_back(in);
  }

  template <typename TFunc>
  void run(const TFunc &func) {
    assert(!m_run);

    for (auto node : m_nodes) node->stat();

    while (m_q.size()) {
      auto node = m_q.front();

      func(node->m_value);
      node->statOuts();

      m_q.pop();
    }
  }

  void dot(std::ostream &os) {
    os << "strict digraph{";

    fun::FAtomStore<Node *, std::size_t> nodeIds;

    for (auto &node : m_nodes) {
      auto id = nodeIds.intern(node);

      os << "n" << id << "[label=\"" << node->m_name << "\"];";

      for (auto &in : node->m_ins) {
        os << "n" << nodeIds.intern(in) << "->n" << id << ";";
      }
    }

    os << "}" << std::endl;
  }

  friend class FMiniDepsGraphNode<T>;
};
} // namespace fpp
