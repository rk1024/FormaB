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

#include "intermedia/instructions.hpp"

namespace fie {
FIDumpFunction::FIDumpFunction(fun::FPtr<FIInputs> inputs, std::ostream &os) :
    m_inputs(inputs),
    m_os(os) {}

void FIDumpFunction::dumpFunc(fun::cons_cell<FIFunctionAtom> args) {
  auto  func = m_inputs->assem()->funcs().value(args.get<0>());
  auto &body = func->body();

  m_os << "\e[1m  Function " << args.get<0>().value() << "\e[0m ("
       << body.instructions.size() << "):" << std::endl;

  for (std::size_t k = 0; k < body.instructions.size(); ++k) {
    m_os << "    ";

    for (std::uint32_t l = 0; l < body.labels.size(); ++l) {
      if (body.labels.value(FILabelAtom(l)).pos() == k)
        m_os << "\e[38;5;7m" << body.labels.value(FILabelAtom(l)).name()
             << ":\e[0m" << std::endl
             << "    ";
    }

    if (body.labels.size()) m_os << "  ";

    auto &ins = body.instructions[k];

    auto &structs = m_inputs->assem()->structs();

    auto tpErrorT = structs.intern(builtins::FIErrorT),
         tpNilT   = structs.intern(builtins::FINilT),
         tpVoidT  = structs.intern(builtins::FIVoidT),
         tpInt8   = structs.intern(builtins::FIInt8),
         tpInt16  = structs.intern(builtins::FIInt16),
         tpInt32  = structs.intern(builtins::FIInt32),
         tpInt64  = structs.intern(builtins::FIInt64),
         tpUint8  = structs.intern(builtins::FIUint8),
         tpUint16 = structs.intern(builtins::FIUint16),
         tpUint32 = structs.intern(builtins::FIUint32),
         tpUint64 = structs.intern(builtins::FIUint64),
         tpFloat  = structs.intern(builtins::FIFloat),
         tpDouble = structs.intern(builtins::FIDouble),
         tpBool   = structs.intern(builtins::FIBool),
         tpString = structs.intern(builtins::FIString);

    m_os << "\e[38;5;4m";

    switch (ins->opcode()) {
    case FIOpcode::Nop: m_os << "nop"; break;

    case FIOpcode::Dup: m_os << "dup"; break;
    case FIOpcode::Pop: m_os << "pop"; break;
    case FIOpcode::Ret: m_os << "ret"; break;

    case FIOpcode::Br:
      m_os << "br\e[0m \e[38;5;7m"
           << body.labels.value(ins.as<FIBranchInstruction>()->label()).name();
      break;
    case FIOpcode::Bez:
      m_os << "bez\e[0m \e[38;5;7m"
           << body.labels.value(ins.as<FIBranchInstruction>()->label()).name();
      break;
    case FIOpcode::Bnz:
      m_os << "bnz\e[0m \e[38;5;7m"
           << body.labels.value(ins.as<FIBranchInstruction>()->label()).name();
      break;

    case FIOpcode::Load: {
      m_os << "load\e[0m \e[38;5;7m"
           << ins.as<FILoadInstructionBase>()->type()->name() << " \e[38;5;1m";

      auto type = structs.intern(ins.as<FILoadInstructionBase>()->type());

      if (type == tpInt8)
        m_os << std::int32_t(ins.as<FILoadInstruction<std::int8_t>>()->value());
      else if (type == tpInt16)
        m_os << ins.as<FILoadInstruction<std::int16_t>>()->value();
      else if (type == tpInt32)
        m_os << ins.as<FILoadInstruction<std::int32_t>>()->value();
      else if (type == tpInt64)
        m_os << ins.as<FILoadInstruction<std::int64_t>>()->value();
      else if (type == tpUint8)
        m_os << std::int32_t(
            ins.as<FILoadInstruction<std::uint8_t>>()->value());
      else if (type == tpUint16)
        m_os << ins.as<FILoadInstruction<std::uint16_t>>()->value();
      else if (type == tpUint32)
        m_os << ins.as<FILoadInstruction<std::uint32_t>>()->value();
      else if (type == tpUint64)
        m_os << ins.as<FILoadInstruction<std::uint64_t>>()->value();
      else if (type == tpFloat)
        m_os << ins.as<FILoadInstruction<float>>()->value();
      else if (type == tpDouble)
        m_os << ins.as<FILoadInstruction<double>>()->value();
      else if (type == tpBool)
        m_os << ins.as<FILoadInstruction<bool>>()->value();
      else if (type == tpString) {
        auto val = ins.as<FILoadInstruction<FIStringAtom>>()->value();
        m_os << val.value() << " \e[38;5;2m\""
             << m_inputs->assem()->strings().value(val) << "\"";
      }
      else if (type == tpErrorT)
        ;
      else
        abort();
      break;
    }

    case FIOpcode::Ldnil: m_os << "ldnil"; break;
    case FIOpcode::Ldvoid: m_os << "ldvoid"; break;

    case FIOpcode::Ldvar:
      m_os << "ldvar\e[0m "
           << body.vars.value(ins.as<FIVarInstruction>()->var()).name();
      break;

    case FIOpcode::Stvar:
      m_os << "stvar\e[0m "
           << body.vars.value(ins.as<FIVarInstruction>()->var()).name();
      break;

    case FIOpcode::Msg:
      m_os << "msg\e[0m \e[38;5;3m"
           << m_inputs->assem()
                  ->msgs()
                  .value(ins.as<FIMessageInstruction>()->msg())
                  .get<1>();
      break;

    case FIOpcode::Tpl:
      m_os << "tpl\e[0m \e[38;5;5m"
           << fun::dumpHex(ins.as<FITupleInstruction>()->count());
      break;

    default: assert(false);
    }

    m_os << "\e[0m" << std::endl;
  }
}
} // namespace fie
