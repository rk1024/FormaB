/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scopeTracker.hpp)
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

#include "object/object.hpp"
#include "ptr.hpp"

namespace fun {
template <typename>
class FScopeTracker;

template <typename T>
class FScopeNode : public FObject {
public:
  using Tracker = FScopeTracker<T>;
  using Node    = FScopeNode<T>;

private:
  Tracker *      m_parent;
  FWeakPtr<Node> m_prev, m_next;
  T              m_value;

public:
  constexpr auto &value() const { return m_value; }

  FScopeNode(Tracker *parent, const T &value) :
      m_parent(parent),
      m_value(value) {}

  ~FScopeNode() {
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

  FPtr<Node> push(const T &value) {
    auto next    = fnew<Node>(m_parent, value);
    next->m_prev = weak(this);

    if (!m_next.good()) m_parent->m_node = weak(next);

    m_next = weak(next);

    return next;
  }

  friend class FScopeTracker<T>;
};

template <typename T>
class FScopeTracker {
public:
  using Node = FScopeNode<T>;

private:
  FWeakPtr<Node> m_node;
  FPtr<Node>     m_rootNode;

public:
  constexpr auto &curr() const { return m_node.lock()->m_value; }

  explicit FScopeTracker(const T &root) : m_rootNode(fnew<Node>(this, root)) {
    m_node = weak(m_rootNode);
  }

  FPtr<Node> move(const T &to) { return m_node.lock()->push(to); }

  friend class FScopeNode<T>;
};
} // namespace fun
