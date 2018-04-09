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

#include <iomanip>

#include "util/dumpHex.hpp"

#include "intermedia/function/instruction.hpp"

namespace fie {
FIDumpFunction::FIDumpFunction(fun::FPtr<FIInputs> inputs, std::ostream &os) :
    m_inputs(inputs),
    m_os(os) {}

void FIDumpFunction::dumpFunc(fun::cons_cell<FIFunctionAtom> args) {
  auto  func = m_inputs->assem()->funcs().value(args.get<0>());
  auto &body = func->body();

  m_os << "\e[1m  Function " << args.get<0>().value() << "\e[0m\n";

  std::unordered_map<fun::FPtr<FIBlock>, std::string> blockNames;
  std::unordered_map<FIRegId, std::string>            regNames;
  std::size_t                                         nameWidth = 0;

  for (int i = 0; i < body.blocks.size(); ++i) {
    blockNames[body.blocks[i]] = "block" + std::to_string(i) + "(" +
                                 body.blocks[i]->name() + ")";

    for (auto ins : body.blocks[i]->body()) {
      std::string name    = std::to_string(ins.id()) + "(" + ins.name() + ")";
      nameWidth           = std::max(nameWidth, name.length());
      regNames[ins.reg()] = name;
    }
  }

  for (auto block : body.blocks) {
    m_os << blockNames.at(block) << ":\n";
    for (auto ins : block->body()) {
      m_os << "  \e[38;5;2m";

      {
        auto flags = m_os.flags();

        m_os << std::setfill(' ') << std::setw(nameWidth) << std::left
             << regNames.at(FIRegId(ins.id()));

        m_os.setf(flags);
      }

      m_os << "\e[0m <- \e[38;5;4m";

      auto &value = ins.value();

      switch (value->opcode()) {
      case FIOpcode::Const: {
        m_os << "const \e[0m";

        auto tp = value.as<const FIConstantBase>()->constType();

        m_os << "\e[38;5;3m"; // << tp->name() << " ";

        switch (value.as<const FIConstantBase>()->constType()) {
          break;
        case FIConstType::Func: {
          auto fn = value.as<const FIConstant<FIFunctionAtom>>()->value();
          m_os << "\e[38;5;5m" << fn.value();
        } break;
        case FIConstType::MsgKw: {
          auto kw = value.as<const FIConstant<FIMessageKeywordAtom>>()->value();
          m_os << "\e[38;5;5m" << kw.value() << " \e[38;5;2m"
               << m_inputs->assem()->keywords().value(kw).name();
        } break;
        case FIConstType::Int8:
          m_os << "\e[38;5;5m"
               << std::int32_t(
                      value.as<const FIConstant<std::int8_t>>()->value());
          break;
        case FIConstType::Int16:
          m_os << "\e[38;5;5m"
               << value.as<const FIConstant<std::int16_t>>()->value();
          break;
        case FIConstType::Int32:
          m_os << "\e[38;5;5m"
               << value.as<const FIConstant<std::int32_t>>()->value();
          break;
        case FIConstType::Int64:
          m_os << "\e[38;5;5m"
               << value.as<const FIConstant<std::int64_t>>()->value();
          break;
        case FIConstType::Uint8:
          m_os << "\e[38;5;5m"
               << std::int32_t(
                      value.as<const FIConstant<std::uint8_t>>()->value());
          break;
        case FIConstType::Uint16:
          m_os << "\e[38;5;5m"
               << value.as<const FIConstant<std::uint16_t>>()->value();
          break;
        case FIConstType::Uint32:
          m_os << "\e[38;5;5m"
               << value.as<const FIConstant<std::uint32_t>>()->value();
          break;
        case FIConstType::Uint64:
          m_os << "\e[38;5;5m"
               << value.as<const FIConstant<std::uint64_t>>()->value();
          break;
        case FIConstType::Float:
          m_os << "\e[38;5;5m" << value.as<const FIConstant<float>>()->value();
          break;
        case FIConstType::Double:
          m_os << "\e[38;5;5m" << value.as<const FIConstant<double>>()->value();
          break;
        case FIConstType::Bool:
          m_os << "\e[38;5;5m" << value.as<const FIConstant<bool>>()->value();
          break;
        case FIConstType::String: {
          auto val = value.as<const FIConstant<FIStringAtom>>()->value();
          m_os << "\e[38;5;5m" << val.value() << " \e[38;5;2m"
               << m_inputs->assem()->strings().value(val);
        }
        }
        break;
      }
      case FIOpcode::Ldvar:
        m_os << "ldvar \e[0m"
             << body.vars.value(value.as<const FILdvarValue>()->var()).name();
        break;
      case FIOpcode::Msg: {
        auto msg = value.as<const FIMsgValue>();

        m_os << "msg   \e[38;5;3m"
             << m_inputs->assem()->msgs().value(msg->msg()).name()
             << "\e[38;5;2m";

        for (auto arg : msg->args()) { m_os << " " << regNames.at(arg); }
        break;
      }
      case FIOpcode::Nil: m_os << "nil"; break;
      case FIOpcode::Phi: {
        m_os << "phi  \e[38;5;2m";
        auto phi = ins.value().as<const FIPhiValue>();
        for (auto &[reg, blk] : phi->values()) {
          m_os << " (" << blockNames.at(blk.lock()) << " -> "
               << regNames.at(reg) << ")";
        }
        break;
      }
      case FIOpcode::Stvar: {
        auto stvar = value.as<const FIStvarValue>();
        m_os << "stvar \e[0m" << body.vars.value(stvar->var()).name()
             << " \e[38;5;2m" << regNames.at(stvar->val());
        break;
      }
      case FIOpcode::Tpl: {
        m_os << "tpl  \e[38;5;2m";
        auto tpl = ins.value().as<const FITplValue>();
        for (auto val : tpl->values()) { m_os << " " << regNames.at(val); }
        break;
      }
      case FIOpcode::Void: m_os << "void"; break;
      }

      m_os << "\e[0m\n";
    }

    m_os << "  \e[38;5;6m";

    switch (block->cont()) {
    case FIBlock::Branch:
      m_os << "btf \e[38;5;2m" << regNames.at(block->ret()) << "\e[0m "
           << blockNames.at(block->contA().lock()) << " "
           << blockNames.at(block->contB().lock());
      break;
    case FIBlock::Return:
      m_os << "ret \e[38;5;2m" << regNames.at(block->ret());
      break;
    case FIBlock::Static:
      m_os << "br \e[0m" << blockNames.at(block->contA().lock());
      break;
    case FIBlock::ERR: m_os << "ERR";
    }

    m_os << "\e[0m\n";
  }
}
} // namespace fie
