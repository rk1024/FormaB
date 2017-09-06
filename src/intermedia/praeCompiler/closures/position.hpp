#pragma once

#include "util/object.hpp"

#include "ast/astBase.hpp"

namespace fie {
namespace pc {
  class PositionTracker;

  class PositionNode : public fun::FObject {
    PositionTracker *           m_parent;
    const frma::FormaAST *      m_curr;
    fun::FWeakPtr<PositionNode> m_prev, m_next;

  public:
    const frma::FormaAST *ast() const { return m_curr; }

    PositionNode(PositionTracker *, const frma::FormaAST *);

    fun::FPtr<PositionNode> push(const frma::FormaAST *);

    virtual ~PositionNode() override;

    friend class PositionTracker;
  };

  class PositionTracker : public fun::FObject {
    fun::FWeakPtr<PositionNode> m_node;
    fun::FPtr<PositionNode>     m_rootNode;

  public:
    const frma::FormaAST *curr() const { return m_node.lock()->m_curr; }

    PositionTracker(const frma::FormaAST *);

    fun::FPtr<PositionNode> move(const frma::FormaAST *);

    friend class PositionNode;
  };
}
}
