/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ti.hpp)
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

#include <cstdint>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "util/ptr.hpp"

#include "_type.hpp"
#include "expr.hpp"
#include "scheme.hpp"

namespace w {

struct TI {
  std::int32_t             supply = 0;
  std::vector<std::string> stack;

  fun::FPtr<const TypeBase> makeVar();

  fun::FPtr<const TypeBase> instantiate(const Scheme &);

  void debugState() const;

  std::string state() const;
};

class TIPos {
  TI *m_t;

public:
  TIPos(TI &t, const std::string &s) : m_t(&t) {
    t.stack.emplace_back(s);
    // t.debugState();
  }

  ~TIPos() { m_t->stack.pop_back(); }
};

fun::FPtr<const TypeBase> ti(const fun::FPtr<const ExprBase> &,
                             const TypeEnv & = TypeEnv());
} // namespace w
