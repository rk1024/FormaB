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

#include "intermedia/function.hpp"
#include "intermedia/messaging/message.hpp"

#include "position.hpp"

namespace fie {
namespace pc {
  class ScopeClosure;

  class FuncClosure : public PositionTracker {
  public:
    using VarIds = std::unordered_map<
        fun::cons_cell<fun::FWeakPtr<ScopeClosure>, std::string>,
        std::uint32_t>;

  private:
    fun::FPtr<ScopeClosure> m_args, m_scope;
    unsigned int            m_nextScopeId = 0;
    FIFunctionBody *        m_body;

  public:
    inline fun::FPtr<ScopeClosure> args() { return m_args; }
    inline fun::FPtr<ScopeClosure> scope() const { return m_scope; }
    inline FIFunctionBody *        body() const { return m_body; }

    FuncClosure(FIFunctionBody &, const frma::FormaAST *);

    FuncClosure &emit(const fun::FPtr<FIInstructionBase> &);
    FuncClosure &emit(fun::FPtr<FIInstructionBase> &&);

    inline FuncClosure &emitBasic(FIOpcode op) {
      return emit(fnew<FIBasicInstruction>(op));
    }

    inline FuncClosure &emitBranch(FIOpcode op, FILabelAtom lbl) {
      return emit(fnew<FIBranchInstruction>(op, lbl));
    }

    template <typename T>
    inline FuncClosure &emitLoad(const T &value) {
      return emit(fnew<FILoadInstruction<T>>(value));
    }

    template <typename T>
    inline FuncClosure &emitLoad(T &&value) {
      return emit(fnew<FILoadInstruction<T>>(value));
    }

    inline FuncClosure &emitMessage(FIMessageAtom msg) {
      return emit(fnew<FIMessageInstruction>(msg));
    }

    inline FuncClosure &emitTuple(std::uint32_t count) {
      return emit(fnew<FITupleInstruction>(count));
    }

    inline FuncClosure &emitVar(FIOpcode op, FIVariableAtom var) {
      return emit(fnew<FIVarInstruction>(op, var));
    }

    FILabelAtom beginLabel();

    void label(FILabelAtom);

    void                    pushScope();
    void                    dropScope();
    fun::FPtr<ScopeClosure> popScope();

    [[noreturn]] void error(std::string &&desc);

    friend class ScopeClosure;
  };
} // namespace pc
} // namespace fie
