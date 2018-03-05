/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (block.cpp)
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

#include "block.hpp"

#include <iostream>
#include <unordered_set>

#include "intermedia/types/builtins.hpp"

namespace fie {
namespace vc {
  bool BlockClosure::assertArity(std::uint32_t arity, const char *name) {
    if (m_stack.size() < arity) {
      std::cerr << "Not enough values on stack for " << name << "."
                << std::endl;

      return false;
    }

    return true;
  }

  void BlockClosure::handlePHOp(std::uint32_t       arity,
                                fun::FPtr<FIStruct> push,
                                const char *        name) {
    bool error = !assertArity(arity, name);

    if (!error) {
      for (std::uint32_t i = 0; i < arity; ++i) m_stack.pop();
    }

    m_stack.push(m_assem->structs().key(error ? builtins::FIErrorT : push));
  }

  void BlockClosure::handleBoolOp(std::uint32_t arity, const char *name) {
    bool error = !assertArity(arity, name);

    for (std::uint32_t i = 0; i < arity; ++i) {
      if (m_stack.top() != m_assem->structs().key(builtins::FIBool)) {
        std::cerr << "Invalid operand " << (i + 1) << " type "
                  << m_assem->structs().value(m_stack.top())->name() << " for "
                  << name << "." << std::endl;
      }

      m_stack.pop();
    }

    m_stack.push(
        m_assem->structs().key(error ? builtins::FIErrorT : builtins::FIBool));
  }

  bool BlockClosure::handlePopBool(const char *error) {
    if (m_stack.top() != m_assem->structs().key(builtins::FIBool)) {
      std::cerr << error << std::endl;

      m_stack.pop();
      return false;
    }

    m_stack.pop();
    return true;
  }

  BlockClosure::BlockClosure(fun::FPtr<FIAssembly>                assem,
                             fun::FPtr<const FIFunction>          func,
                             std::queue<fun::FPtr<BlockClosure>> *q,
                             std::unordered_set<std::size_t> *    checked) :
      m_assem(assem),
      m_func(func),
      m_q(q),
      m_checked(checked),
      m_vars(func->args()) {}

  BlockClosure::BlockClosure(fun::FPtr<BlockClosure> block, std::size_t pc) :
      m_assem(block->m_assem),
      m_func(block->m_func),
      m_q(block->m_q),
      m_checked(block->m_checked),
      m_pc(pc),
      m_stack(block->m_stack),
      m_vars(block->m_vars) {}

  void BlockClosure::iterate() {
    auto body = m_func->body();
    enum { Move, Stay, Stop } action;

    std::unordered_set<std::size_t> covered;

    while (m_pc < body.instructions.size()) {
      if (!covered.insert(m_pc).second) break;

      action = Move;

      auto &ins = body.instructions.at(m_pc);

      switch (ins->opcode()) {
      case FIOpcode::Nop: break;

      case FIOpcode::Dup: m_stack.push(m_stack.top()); break;
      case FIOpcode::Pop: m_stack.pop(); break;
      case FIOpcode::Ret:
        if (m_stack.size() != 1)
          std::cerr << "bad return stack size (" << m_stack.size() << ")"
                    << std::endl;
        action = Stop;
        break;

      case FIOpcode::Br:
        m_pc = body.labels.value(ins.as<FIBranchInstruction>()->label()).pos();
        action = Stay;
        break;

      case FIOpcode::Bez:
      case FIOpcode::Bnz: {
        handlePopBool("Invalid condition type for branch.");
        std::size_t addr = body.labels
                               .value(ins.as<FIBranchInstruction>()->label())
                               .pos();
        if (m_checked->insert(addr).second)
          m_q->emplace(fnew<BlockClosure>(fun::wrap(this), addr));
        break;
      }

      case FIOpcode::Load:
        m_stack.push(
            m_assem->structs().key(ins.as<FILoadInstructionBase>()->type()));
        break;

      case FIOpcode::Ldnil:
        m_stack.push(m_assem->structs().key(builtins::FINilT));
        break;
      case FIOpcode::Ldvoid:
        m_stack.push(m_assem->structs().key(builtins::FIVoidT));
        break;

      case FIOpcode::Ldvar: {
        auto it = m_vars.find(ins.as<FIVarInstruction>()->var());

        if (it == m_vars.end()) {
          std::cerr << "load-before-store ("
                    << body.vars.value(ins.as<FIVarInstruction>()->var()).name()
                    << ")" << std::endl;
          m_stack.push(m_assem->structs().key(builtins::FIErrorT));
        }
        else
          m_stack.push(it->second);
        break;
      }

      case FIOpcode::Stvar:
        if (m_vars.find(ins.as<FIVarInstruction>()->var()) != m_vars.end())
          std::cerr << "variable reassignment ("
                    << body.vars.value(ins.as<FIVarInstruction>()->var()).name()
                    << ")" << std::endl;

        m_vars[ins.as<FIVarInstruction>()->var()] = m_stack.top();
        m_stack.pop();
        break;

      case FIOpcode::Msg:
        handlePHOp(m_assem->msgs()
                       .value(ins.as<FIMessageInstruction>()->msg())
                       .get<0>(),
                   builtins::FIVoidT,
                   "message");
        break;

      case FIOpcode::Tpl:
        handlePHOp(ins.as<FITupleInstruction>()->count(),
                   builtins::FIVoidT,
                   "tuple");
        break;
      }

      switch (action) {
      case Move: ++m_pc; break;
      case Stay: break;
      case Stop: return;
      default: assert(false);
      }
    }
  }
} // namespace vc
} // namespace fie
