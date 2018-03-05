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

  FuncClosure &FuncClosure::emit(const fun::FPtr<FIInstructionBase> &ins) {
    m_body->instructions.emplace_back(ins);
    return *this;
  }

  FuncClosure &FuncClosure::emit(fun::FPtr<FIInstructionBase> &&ins) {
    m_body->instructions.emplace_back(ins);
    return *this;
  }

  FILabelAtom FuncClosure::beginLabel() {
    return m_body->labels.emplace(static_cast<std::uint32_t>(-1),
                                  "l" + std::to_string(m_body->labels.size()));
  }

  void FuncClosure::label(FILabelAtom id) {
    m_body->labels.value(id).pos() = static_cast<std::uint32_t>(
        m_body->instructions.size());
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

  void FuncClosure::error(std::string &&desc) {
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
} // namespace pc
} // namespace fie
