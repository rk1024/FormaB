/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (depsGraph.cpp)
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

#include "depsGraph.hpp"

#include <iostream>

#include "util/atom.hpp"
#include "util/dumpHex.hpp"

namespace fpp {
void FDepsGraphNode::statSelf() {
  if (m_ready) return;

  for (auto in : m_ins) {
    if (in.lock()->m_state != FDepsGraphEdge::Done) return;
  }

  m_ready = true;
}

void FDepsGraphNode::stat() {
  if (m_ready) return;

  for (auto in : m_ins) {
    if (in.lock()->m_state != FDepsGraphEdge::Done) return;
  }

  m_ready = true;

  for (auto out : m_outs) out.lock()->stat();
}

FDepsGraphNode::FDepsGraphNode(const std::string &name) : m_name(name) {}

void FDepsGraphEdge::stat() {
  switch (m_state) {
  case Done:
  case Open: return;
  case Closed: break;
  default: assert(false);
  }

  for (auto in : m_ins) {
    if (!in.lock()->m_ready) return;
  }

  m_state = Open;

  m_graph.lock()->m_q.push(fun::weak(this));
}

void FDepsGraphEdge::run() {
#if !defined(NDEBUG)
  assert(m_state == Open);

  assert(m_ins.size());
  assert(m_outs.size());

  for (auto in : m_ins) assert(in.lock()->m_ready);
  for (auto out : m_outs) assert(!out.lock()->m_ready);
#endif

  m_rule->run();
  m_state = Done;

  for (auto out : m_outs) out.lock()->stat();
}

FDepsGraphEdge::FDepsGraphEdge(const std::string &           name,
                               fun::FWeakPtr<FDepsGraph>     graph,
                               fun::FPtr<FDataGraphRuleBase> rule) :
    m_name(name),
    m_graph(graph),
    m_rule(rule) {}

void FDepsGraph::run(const fdi::FLogger &logger) {
  assert(!m_run);

  for (auto node : m_nodes) node->statSelf();

  for (auto edge : m_edges) edge->stat();

  if (m_q.empty()) logger.warn("depsgraph", "nothing to do");

  while (m_q.size()) {
    auto edge = m_q.front().lock();


    std::cerr
#if !defined(DEBUG)
        << "\r\e[2K"
#endif
        << edge->m_name;

    {
      bool first = true;
      for (auto out : edge->m_outs) {
        if (first) {
          std::cerr << " ";
          first = false;
        }
        else
          std::cerr << ", ";

        std::cerr << "\e[38;5;6m" << out.lock()->m_name << "\e[0m";
      }

#if defined(DEBUG)
      std::cerr << std::endl;
#endif
    }

    edge->run();

    m_q.pop();
  }

#if !defined(DEBUG)
  std::cerr << "\r\e[2K";
#endif

  m_run = true;
}

void FDepsGraph::dot(std::ostream &os) {
  os << "strict digraph{";

  fun::FAtomStore<fun::FPtr<FDepsGraphNode>, std::size_t> nodeIds;
  fun::FAtomStore<fun::FPtr<FDepsGraphEdge>, std::size_t> edgeIds;

  for (auto node : m_nodes)
    os << "n" << nodeIds.intern(node).value() << "[label=\"" << node->m_name
       << "\"];";
  for (auto edge : m_edges)
    os << "e" << edgeIds.intern(edge).value() << "[label=\"" << edge->m_name
       << "\"];";

  for (auto edge : m_edges) {
    auto id = edgeIds.intern(edge);

    for (auto in : edge->m_ins)
      os << "n" << nodeIds.intern(in.lock()).value() << "->e" << id.value()
         << ";";

    for (auto out : edge->m_outs)
      os << "e" << id.value() << "->n" << nodeIds.intern(out.lock()).value()
         << ";";
  }

  os << "}" << std::endl;
}
} // namespace fpp
