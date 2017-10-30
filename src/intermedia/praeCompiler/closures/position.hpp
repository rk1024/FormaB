/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (position.hpp)
* Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
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

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "ast/astBase.hpp"

namespace fie {
namespace pc {
  class PositionTracker;

  class PositionNode : public fun::FObject {
    PositionTracker *           m_parent;
    const frma::FormaAST *      m_curr;
    fun::FWeakPtr<PositionNode> m_prev, m_next; // TODO: Maybe stack allocate?

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
