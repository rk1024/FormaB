/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (dumpFunction.cpp)
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

#include "dumpFunction.hpp"

#include "util/dumpHex.hpp"

namespace fie {
FIDumpFunction::FIDumpFunction(fun::FPtr<FIInputs> inputs, std::ostream &os) :
    m_inputs(inputs),
    m_os(os) {}

void FIDumpFunction::dumpFunc(fun::cons_cell<std::uint32_t> args) {
  auto func = m_inputs->assem()->funcs().value(args.get<0>());
  auto body = func->body();

  m_os << "\e[1m  Function " << args.get<0>() << "\e[0m ("
       << body.instructions.size() << "):" << std::endl;

  for (std::size_t k = 0; k < body.instructions.size(); ++k) {
    m_os << "    ";

    for (std::uint32_t l = 0; l < body.labels.size(); ++l) {
      if (body.labels.at(l).pos() == k)
        m_os << "\e[38;5;7m" << body.labels.at(l).name() << ":\e[0m"
             << std::endl
             << "    ";
    }

    if (body.labels.size()) m_os << "  ";

    auto ins = body.instructions.at(k);

    m_os << "\e[38;5;4m";

    switch (ins.op) {
    case FIOpcode::Nop: m_os << "nop"; break;

    case FIOpcode::Dup: m_os << "dup"; break;
    case FIOpcode::Pop: m_os << "pop"; break;
    case FIOpcode::Ret: m_os << "ret"; break;

    case FIOpcode::Br:
      m_os << "br\e[0m \e[38;5;7m"
           << (ins.br.lbl ? body.labels.at(ins.br.id).name()
                          : std::to_string(ins.br.addr));
      break;
    case FIOpcode::Bez:
      m_os << "bez\e[0m \e[38;5;7m"
           << (ins.br.lbl ? body.labels.at(ins.br.id).name()
                          : std::to_string(ins.br.addr));
      break;
    case FIOpcode::Bnz:
      m_os << "bnz\e[0m \e[38;5;7m"
           << (ins.br.lbl ? body.labels.at(ins.br.id).name()
                          : std::to_string(ins.br.addr));
      break;

    case FIOpcode::Ldci4: m_os << "ldci4\e[0m \e[38;5;5m" << ins.i4; break;
    case FIOpcode::Ldci8: m_os << "ldci8\e[0m \e[38;5;5m" << ins.i8; break;
    case FIOpcode::Ldcr4: m_os << "ldcr4\e[0m \e[38;5;5m" << ins.r4; break;
    case FIOpcode::Ldcr8: m_os << "ldcr8\e[0m \e[38;5;5m" << ins.r8; break;

    case FIOpcode::Ldnil: m_os << "ldnil"; break;
    case FIOpcode::Ldvoid: m_os << "ldvoid"; break;

    case FIOpcode::Ldvar:
      m_os << "ldvar\e[0m " << body.vars.value(ins.u4).name();
      break;

    case FIOpcode::Ldstr:
      m_os << "ldstr\e[0m \e[38;5;6m\""
           << m_inputs->assem()->strings().value(ins.u4) << "\"";
      break;
    case FIOpcode::Ldfun:
      m_os << "ldfun\e[0m \e[38;5;2m" << fun::dumpHex(ins.u4);
      break;
    case FIOpcode::Ldkw:
      m_os << "ldkw\e[0m \e[38;5;3m"
           << m_inputs->assem()->keywords().value(ins.u4).name();
      break;

    case FIOpcode::Stvar:
      m_os << "stvar\e[0m " << body.vars.value(ins.u4).name();
      break;

    case FIOpcode::Cvi1: m_os << "cvi1"; break;
    case FIOpcode::Cvi2: m_os << "cvi2"; break;
    case FIOpcode::Cvi4: m_os << "cvi4"; break;
    case FIOpcode::Cvi8: m_os << "cvi8"; break;
    case FIOpcode::Cvu1: m_os << "cvu1"; break;
    case FIOpcode::Cvu2: m_os << "cvu2"; break;
    case FIOpcode::Cvu4: m_os << "cvu4"; break;
    case FIOpcode::Cvu8: m_os << "cvu8"; break;
    case FIOpcode::Cvr4: m_os << "cvr4"; break;
    case FIOpcode::Cvr8: m_os << "cvr8"; break;

    case FIOpcode::Msg:
      m_os << "msg\e[0m \e[38;5;3m"
           << m_inputs->assem()->msgs().value(ins.u4).get<1>();
      break;

    case FIOpcode::Tpl:
      m_os << "tpl\e[0m \e[38;5;5m" << fun::dumpHex(ins.u4);
      break;

    default: assert(false);
    }

    m_os << "\e[0m" << std::endl;
  }
}
} // namespace fie
