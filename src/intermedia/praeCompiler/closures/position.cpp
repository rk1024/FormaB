#include "position.hpp"

using namespace frma;

namespace fie {
namespace pc {
  PositionTracker::PositionTracker(const FormaAST *curr)
      : m_rootNode(fnew<PositionNode>(this, curr)) {
    m_node = fun::weak(m_rootNode);
  }

  fun::FPtr<PositionNode> PositionTracker::move(const frma::FormaAST *to) {
    return m_node.lock()->push(to);
  }

  PositionNode::PositionNode(PositionTracker *     parent,
                             const frma::FormaAST *curr)
      : m_parent(parent), m_curr(curr) {}

  fun::FPtr<PositionNode> PositionNode::push(const frma::FormaAST *node) {
    auto next    = fnew<PositionNode>(m_parent, node);
    next->m_prev = fun::weak(this);

    if (!m_next) m_parent->m_node = fun::weak(next);

    m_next = fun::weak(next);

    return next;
  }

  PositionNode::~PositionNode() {
    if (m_prev) {
      auto prev = m_prev.lock();
      assert(prev->m_next.peek() == this);
      prev->m_next = m_next;
    }
    if (m_next) {
      auto next = m_next.lock();
      assert(next->m_prev.peek() == this);
      next->m_prev = m_prev;
    } else
      m_parent->m_node = m_prev;
  }
}
}
