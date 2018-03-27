/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (function.cpp)
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

#include "function.hpp"

#include "util/compilerError.hpp"

#include "scope.hpp"

namespace fie {
namespace pc {
  FuncClosure::FuncClosure(FIFunctionBody &body, const frma::FormaAST *curr) :
      PositionTracker(curr),
      m_args(fnew<ScopeClosure>(true, fun::weak(this), nullptr)),
      m_body(&body) {
    m_scope = m_args;
    pushScope();
  }

  void FuncClosure::pushScope() {
    m_scope = fnew<ScopeClosure>(false, fun::weak(this), m_scope);
  }

  void FuncClosure::dropScope() { m_scope = m_scope->parent(); }

  fun::FPtr<ScopeClosure> FuncClosure::popScope() {
    auto ret = std::move(m_scope);
    m_scope  = ret->parent();
    return ret;
  }
} // namespace pc
} // namespace fie
