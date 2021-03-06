/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (verifier.cpp)
* Copyright (C) 2017 Ryan Schroeder, Colin Unger
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

#include "verifier.hpp"

#include <iostream>
#include <queue>
#include <stack>
#include <unordered_set>

#include "util/cons.hpp"

#include "closures/block.hpp"

using namespace fie::vc;

namespace fie {
FIVerifier::FIVerifier(fun::FPtr<FIInputs> inputs) : m_inputs(inputs) {}

void FIVerifier::verifyFunc(fun::cons_cell<std::uint32_t> args) {
  auto func = m_inputs->assem()->funcs().value(args.get<0>());
  std::queue<fun::FPtr<BlockClosure>> q;
  q.push(fnew<BlockClosure>(m_inputs->assem(), func, &q));

  while (q.size()) {
    q.front()->iterate();
    q.pop();
  }

  // std::unordered_set<std::size_t> startSet;
  // std::queue<fun::cons_cell<std::size_t, std::int32_t>> startQ;
  // startQ.emplace(0, 0);

  // while (startQ.size()) {
  //   auto start = startQ.front();

  //   if (startSet.insert(start.get<0>()).second) {
  //     std::int32_t stackLevel = start.get<1>();

  //     std::unordered_set<std::size_t> covered;

  //     for (std::size_t pc = start.get<0>();
  //          pc < func->body().instructions.size();) {
  //       auto ins = func->body().instructions.at(pc);
  //       enum {
  //         Increment,
  //         NoIncrement,
  //         Stop,
  //       } action = Increment;

  //       if (!covered.insert(pc).second)
  //         break; // Don't get caught in an infinite loop

  //       switch (ins.op) {
  //       case FIOpcode::Nop: break;

  //       case FIOpcode::Dup: ++stackLevel; break;
  //       case FIOpcode::Pop: --stackLevel; break;
  //       case FIOpcode::Ret:
  //         if (stackLevel > 1)
  //           std::cerr << "bad return stack level (" << stackLevel << ")"
  //                     << std::endl;
  //         action = Stop;
  //         break;

  //       case FIOpcode::Add: --stackLevel; break;
  //       case FIOpcode::Sub: --stackLevel; break;
  //       case FIOpcode::Mul: --stackLevel; break;
  //       case FIOpcode::Div: --stackLevel; break;
  //       case FIOpcode::Mod: --stackLevel; break;

  //       case FIOpcode::Neg: break;
  //       case FIOpcode::Pos: break;

  //       case FIOpcode::Ceq: --stackLevel; break;
  //       case FIOpcode::Cgt: --stackLevel; break;
  //       case FIOpcode::Cgtu: --stackLevel; break;
  //       case FIOpcode::Clt: --stackLevel; break;
  //       case FIOpcode::Cltu: --stackLevel; break;

  //       case FIOpcode::Con: --stackLevel; break;
  //       case FIOpcode::Dis: --stackLevel; break;

  //       case FIOpcode::Inv: break;

  //       case FIOpcode::Br:
  //         if (ins.br.lbl)
  //           pc = func->body().labels.at(ins.br.id).pos;
  //         else
  //           pc += ins.br.addr;
  //         action = NoIncrement;
  //         break;

  //       case FIOpcode::Bez:
  //       case FIOpcode::Bnz:
  //         --stackLevel;
  //         startQ.emplace(ins.br.lbl ? func->body().labels.at(ins.br.id).pos :
  //                                     pc + ins.br.addr,
  //                        stackLevel);

  //         break;

  //       case FIOpcode::Ldci4:
  //       case FIOpcode::Ldci8:
  //       case FIOpcode::Ldcr4:
  //       case FIOpcode::Ldcr8: ++stackLevel; break;

  //       case FIOpcode::Ldnil:
  //       case FIOpcode::Ldvoid: ++stackLevel; break;

  //       case FIOpcode::Ldvar: ++stackLevel; break;

  //       case FIOpcode::Ldstr:
  //       case FIOpcode::Ldfun: ++stackLevel; break;

  //       case FIOpcode::Stvar: --stackLevel; break;

  //       case FIOpcode::Cvi1: break;
  //       case FIOpcode::Cvi2: break;
  //       case FIOpcode::Cvi4: break;
  //       case FIOpcode::Cvi8: break;
  //       case FIOpcode::Cvu1: break;
  //       case FIOpcode::Cvu2: break;
  //       case FIOpcode::Cvu4: break;
  //       case FIOpcode::Cvu8: break;
  //       case FIOpcode::Cvr4: break;
  //       case FIOpcode::Cvr8: break;

  //       case FIOpcode::Msg:
  //         stackLevel -= m_assem->msgs().value(ins.u4).get<0>();
  //         ++stackLevel; // Because unsigned numbers
  //         break;

  //       case FIOpcode::Tpl:
  //         stackLevel -= ins.u4;
  //         ++stackLevel; // Because unsigned numbers
  //         break;
  //       }

  //       switch (action) {
  //       case NoIncrement: break;
  //       case Increment: ++pc; break;
  //       case Stop: goto stop;
  //       }

  //       continue;
  //     stop:
  //       break;
  //     }
  //   }
  //   startQ.pop();
  // }

  // propagate(func);
}
}
