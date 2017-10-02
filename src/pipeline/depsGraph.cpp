#include "depsGraph.hpp"

#include "util/atom.hpp"
#include "util/dumpHex.hpp"

namespace fps {
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
                               fun::FPtr<FDataGraphRuleBase> rule)
    : m_name(name), m_graph(graph), m_rule(rule) {}

fun::FPtr<FDepsGraphNode> FDepsGraph::node(const std::string &name) {
  auto node = fnew<FDepsGraphNode>(name);
  m_nodes.push_back(node);
  return node;
}

fun::FPtr<FDepsGraphEdge> FDepsGraph::edge(const std::string &           name,
                                           fun::FPtr<FDataGraphRuleBase> rule) {
  auto edge = fnew<FDepsGraphEdge>(name, fun::weak(this), rule);
  m_edges.push_back(edge);
  return edge;
}

void FDepsGraph::run() {
  assert(!m_run);

  for (auto node : m_nodes) node->statSelf();

  for (auto edge : m_edges) edge->stat();

  if (m_q.empty()) std::cerr << "WARNING: Nothing to do." << std::endl;

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
        } else
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

  fun::FAtomStore<fun::FPtr<FDepsGraphNode>> nodeIds;
  fun::FAtomStore<fun::FPtr<FDepsGraphEdge>> edgeIds;

  for (auto node : m_nodes)
    os << "n" << nodeIds.intern(node) << "[label=\"" << node->m_name << "\"];";
  for (auto edge : m_edges)
    os << "e" << edgeIds.intern(edge) << "[label=\"" << edge->m_name << "\"];";

  for (auto edge : m_edges) {
    auto id = edgeIds.intern(edge);

    for (auto in : edge->m_ins)
      os << "n" << nodeIds.intern(in.lock()) << "->e" << id << ";";

    for (auto out : edge->m_outs)
      os << "e" << id << "->n" << nodeIds.intern(out.lock()) << ";";
  }

  os << "}" << std::endl;
}
}
