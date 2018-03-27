/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (position.cpp)
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

#include "position.hpp"

#include "util/compilerError.hpp"

using namespace frma;

namespace fie {
namespace pc {
  PositionTracker::PositionTracker(const FormaAST *curr) :
      m_rootNode(fnew<PositionNode>(this, curr)) {
    m_node = fun::weak(m_rootNode);
  }

  fun::FPtr<PositionNode> PositionTracker::move(const frma::FormaAST *to) {
    return m_node.lock()->push(to);
  }

  void PositionTracker::error(std::string &&desc) const {
    auto loc = curr()->loc();

    std::ostringstream os;

    os << "\x1b[1m";

    if (loc.begin.filename)
      os << *loc.begin.filename;
    else
      os << "???";


    os << ":" << loc.begin.line << ":" << loc.begin.column;

    if (loc.end != loc.begin) os << "-";

    if (loc.end.filename != loc.begin.filename) {
      if (loc.end.filename)
        os << *loc.end.filename;
      else
        os << "???";

      os << ":";

      goto diffLine;
    }
    else if (loc.end.line != loc.begin.line) {
    diffLine:
      os << loc.end.line << ":";

      goto diffCol;
    }
    else if (loc.end.column != loc.begin.column) {
    diffCol:
      os << loc.end.column;
    }

    os << ": \x1b[38;5;9merror:\x1b[0m " << desc << std::endl;

    std::cerr << os.str();

    throw fun::compiler_error();
  }

  PositionNode::PositionNode(PositionTracker *     parent,
                             const frma::FormaAST *curr) :
      m_parent(parent),
      m_curr(curr) {}

  fun::FPtr<PositionNode> PositionNode::push(const frma::FormaAST *node) {
    auto next    = fnew<PositionNode>(m_parent, node);
    next->m_prev = fun::weak(this);

    if (!m_next.good()) m_parent->m_node = fun::weak(next);

    m_next = fun::weak(next);

    return next;
  }

  PositionNode::~PositionNode() {
    if (auto prev = m_prev.lockOrNull(); !prev.nil()) {
      assert(prev->m_next.peek() == this);
      prev->m_next = m_next;
    }
    if (auto next = m_next.lockOrNull(); !next.nil()) {
      assert(next->m_prev.peek() == this);
      next->m_prev = m_prev;
    }
    else
      m_parent->m_node = m_prev;
  }
} // namespace pc
} // namespace fie
