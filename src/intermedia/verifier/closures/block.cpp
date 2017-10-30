/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (block.cpp)
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
                             std::queue<fun::FPtr<BlockClosure>> *q)
      : m_assem(assem), m_func(func), m_q(q), m_vars(func->args()) {}

  BlockClosure::BlockClosure(fun::FPtr<BlockClosure> block, std::size_t pc)
      : m_assem(block->m_assem),
        m_func(block->m_func),
        m_q(block->m_q),
        m_pc(pc),
        m_stack(block->m_stack),
        m_vars(block->m_vars) {}

  void BlockClosure::iterate() {
    auto body = m_func->body();
    enum { Move, Stay, Stop } action = Move;

    std::unordered_set<std::size_t> covered;

    while (m_pc < body.instructions.size()) {
      if (!covered.insert(m_pc).second) break;

      auto ins = body.instructions.at(m_pc);

      switch (ins.op) {
      case FIOpcode::Nop: break;

      case FIOpcode::Dup: m_stack.push(m_stack.top()); break;
      case FIOpcode::Pop: m_stack.pop(); break;
      case FIOpcode::Ret:
        if (m_stack.size() != 1)
          std::cerr << "bad return stack size (" << m_stack.size() << ")"
                    << std::endl;
        action = Stop;
        break;

      // case FIOpcode::Add: handlePHOp(2, builtins::FIErrorT, "addition");
      // break;
      // case FIOpcode::Sub:
      //   handlePHOp(2, builtins::FIErrorT, "subtraction");
      //   break;
      // case FIOpcode::Mul:
      //   handlePHOp(2, builtins::FIErrorT, "multiplication");
      //   break;
      // case FIOpcode::Div: handlePHOp(2, builtins::FIErrorT, "division");
      // break;
      // case FIOpcode::Mod: handlePHOp(2, builtins::FIErrorT, "modulo"); break;

      // case FIOpcode::Neg: handlePHOp(1, builtins::FIErrorT, "negation");
      // break;
      // case FIOpcode::Pos: handlePHOp(1, builtins::FIErrorT, "identity");
      // break;

      // case FIOpcode::Ceq:
      //   handlePHOp(2, builtins::FIErrorT, "equality comparison");
      //   break;
      // case FIOpcode::Cgt:
      //   handlePHOp(2, builtins::FIBool, "greater-than comparison");
      //   break;
      // case FIOpcode::Cgtu:
      //   handlePHOp(2, builtins::FIBool, "unsigned greater-than comparison");
      //   break;
      // case FIOpcode::Clt:
      //   handlePHOp(2, builtins::FIBool, "less-than comparison");
      //   break;
      // case FIOpcode::Cltu:
      //   handlePHOp(2, builtins::FIBool, "unsigned less-than comparison");
      //   break;

      // case FIOpcode::Con: handleBoolOp(2, "conjunction"); break;
      // case FIOpcode::Dis: handleBoolOp(2, "disjunction"); break;

      // case FIOpcode::Inv: handleBoolOp(1, "inversion"); break;

      case FIOpcode::Br:
        if (ins.br.lbl)
          m_pc = body.labels.at(ins.br.id).pos();
        else
          m_pc += ins.br.addr;
        action = Stay;
        break;

      case FIOpcode::Bez:
      case FIOpcode::Bnz:
        handlePopBool("Invalid condition type for branch.");
        m_q->emplace(fnew<BlockClosure>(
            fun::wrap(this),
            ins.br.lbl ? body.labels.at(ins.br.id).pos() : m_pc + ins.br.addr));
        break;

      case FIOpcode::Ldci4:
        m_stack.push(m_assem->structs().value(builtins::FIInt8));
        break;
      case FIOpcode::Ldci8:
        m_stack.push(m_assem->structs().value(builtins::FIInt8));
        break;
      case FIOpcode::Ldcr4:
        m_stack.push(m_assem->structs().value(builtins::FIInt8));
        break;
      case FIOpcode::Ldcr8:
        m_stack.push(m_assem->structs().value(builtins::FIInt8));
        break;

      case FIOpcode::Ldnil:
        m_stack.push(m_assem->structs().value(builtins::FINilT));
        break;
      case FIOpcode::Ldvoid:
        m_stack.push(m_assem->structs().value(builtins::FIVoidT));
        break;

      case FIOpcode::Ldvar: {
        auto it = m_vars.find(ins.u4);

        if (it == m_vars.end()) {
          std::cerr << "load-before-store (" << body.vars.value(ins.u4).name()
                    << ")" << std::endl;
          m_stack.push(m_assem->structs().value(builtins::FIErrorT));
        } else
          m_stack.push(it->second);
        break;
      }

      case FIOpcode::Ldstr:
        m_stack.push(m_assem->structs().key(builtins::FIString));
        break;
      case FIOpcode::Ldfun:
        m_stack.push(m_assem->structs().key(builtins::FIErrorT));
        break;
      case FIOpcode::Ldkw:
        m_stack.push(m_assem->structs().key(builtins::FIErrorT));
        break;

      case FIOpcode::Stvar:
        if (m_vars.count(ins.u4))
          std::cerr << "variable reassignment ("
                    << body.vars.value(ins.u4).name() << ")" << std::endl;

        m_vars[ins.u4] = m_stack.top();
        m_stack.pop();
        break;

      case FIOpcode::Cvi1: handlePHOp(1, builtins::FIInt8, "cast"); break;
      case FIOpcode::Cvi2: handlePHOp(1, builtins::FIInt16, "cast"); break;
      case FIOpcode::Cvi4: handlePHOp(1, builtins::FIInt32, "cast"); break;
      case FIOpcode::Cvi8: handlePHOp(1, builtins::FIInt64, "cast"); break;
      case FIOpcode::Cvu1: handlePHOp(1, builtins::FIUint8, "cast"); break;
      case FIOpcode::Cvu2: handlePHOp(1, builtins::FIUint16, "cast"); break;
      case FIOpcode::Cvu4: handlePHOp(1, builtins::FIUint32, "cast"); break;
      case FIOpcode::Cvu8: handlePHOp(1, builtins::FIUint64, "cast"); break;
      case FIOpcode::Cvr4: handlePHOp(1, builtins::FIFloat, "cast"); break;
      case FIOpcode::Cvr8: handlePHOp(1, builtins::FIDouble, "cast"); break;

      case FIOpcode::Msg:
        handlePHOp(m_assem->msgs().value(ins.u4).get<0>(),
                   builtins::FIVoidT,
                   "message");
        break;
      // case FIOpcode::Curry:
      //   handlePHOp(m_assem->msgs().value(ins.u4).get<0>(),
      //              builtins::FIErrorT,
      //              "curry");
      //   break;

      case FIOpcode::Tpl: handlePHOp(ins.u4, builtins::FIVoidT, "tuple"); break;
      }

      switch (action) {
      case Move: ++m_pc; break;
      case Stay: break;
      case Stop: return;
      default: assert(false);
      }
    }
  }
}
}
