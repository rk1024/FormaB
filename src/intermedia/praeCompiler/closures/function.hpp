/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (function.hpp)
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

#include <unordered_map>

#include "util/cons.hpp"

#include "intermedia/function/function.hpp"
#include "intermedia/messaging/message.hpp"

#include "position.hpp"

namespace fie {
namespace pc {
  class ScopeClosure;
  class BlockClosure;

  class FuncClosure : public PositionTracker {
  public:
    using VarIds = std::unordered_map<
        fun::cons_cell<fun::FWeakPtr<ScopeClosure>, std::string>,
        std::uint32_t>;

  private:
    fun::FPtr<ScopeClosure> m_args, m_scope;
    std::uint32_t           m_nextScopeId = 0, m_nextRegId = 0;
    FIFunctionBody *        m_body;

    auto regId() { return m_nextRegId++; }

  public:
    constexpr auto &args() const { return m_args; }
    constexpr auto &scope() const { return m_scope; }
    constexpr auto &body() const { return m_body; }

    FuncClosure(FIFunctionBody &, const frma::FormaAST *);

    void                    pushScope();
    void                    dropScope();
    fun::FPtr<ScopeClosure> popScope();

    friend class ScopeClosure;
    friend class BlockClosure;
  };
} // namespace pc
} // namespace fie
