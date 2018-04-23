/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (context.hpp)
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

#include "ast/astBase.hpp"

#include "util/scopeTracker.hpp"

namespace pre::cc {
class CompileContext {
  fun::FScopeTracker<const fps::FASTBase *> m_node;

public:
  constexpr auto &node() const { return m_node.curr(); }
  constexpr auto &loc() const { return m_node.curr()->loc(); }

  explicit CompileContext(const fps::FASTBase *pos) : m_node(pos) {}

  decltype(auto) move(const fps::FASTBase *to) { return m_node.move(to); }
};
} // namespace pre::cc
