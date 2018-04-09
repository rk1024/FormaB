/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (values.cpp)
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

#include "values.hpp"

#include <sstream>

#include "intermedia/messaging/builtins.hpp"
#include "intermedia/typeSolver/ti.hpp"

namespace fie {
static std::string opcodeName(FIOpcode op) {
  switch (op) {
  case FIOpcode::Const: return "const";
  case FIOpcode::Ldvar: return "ldvar";
  case FIOpcode::Msg: return "msg";
  case FIOpcode::Nil: return "nil";
  case FIOpcode::Phi: return "phi";
  case FIOpcode::Stvar: return "stvar";
  case FIOpcode::Tpl: return "tpl";
  case FIOpcode::Void: return "void";
  }
}

std::string FIValue::to_string() const {
  std::ostringstream oss;

  oss << "\e[38;5;6m[" << opcodeName(opcode()) << "] \e[0m"
      << m_pos->toString();

  auto &loc = m_pos->loc();

  oss << "\e[38;5;8m at ";

  if (loc.begin.filename)
    oss << *loc.begin.filename;
  else
    oss << "???";


  oss << ":" << loc.begin.line << ":" << loc.begin.column;

  if (loc.end != loc.begin) oss << "-";

  if (loc.end.filename != loc.begin.filename) {
    if (loc.end.filename)
      oss << *loc.end.filename;
    else
      oss << "???";

    oss << ":";

    goto diffLine;
  }
  else if (loc.end.line != loc.begin.line) {
  diffLine:
    oss << loc.end.line << ":";

    goto diffCol;
  }
  else if (loc.end.column != loc.begin.column) {
  diffCol:
    oss << loc.end.column;
  }

  oss << "\e[0m";

  return oss.str();
}

#define TIFUNC(type) type::TIResult type::tiImpl([[maybe_unused]] TI &t) const

FIOpcode FIOpValue::opcode() const { return m_opcode; }

std::vector<FIRegId> FIOpValue::deps() const { return {}; }

TIFUNC(FIOpValue) {
  switch (m_opcode) {
  case FIOpcode::Nil:
    return TIResult(w::Subst(),
                    w::Constraints(),
                    fnew<WTypeStruct>(builtins::FIErrorT,
                                      WTypeStruct::Params()));
  case FIOpcode::Void:
    return TIResult(w::Subst(),
                    w::Constraints(),
                    fnew<WTypeStruct>(builtins::FIVoidT,
                                      WTypeStruct::Params()));
  default: abort();
  }
}

FIOpcode FIConstantBase::opcode() const { return FIOpcode::Const; }

std::vector<FIRegId> FIConstantBase::deps() const { return {}; }

#define CONST_TIFUNC(_tp, _struct)                                             \
  template <>                                                                  \
  TIFUNC(FIConstant<_tp>) {                                                    \
    return TIResult(w::Subst(),                                                \
                    w::Constraints(),                                          \
                    fnew<WTypeStruct>(_struct, WTypeStruct::Params()));        \
  }

CONST_TIFUNC(bool, builtins::FIBool)

CONST_TIFUNC(float, builtins::FIFloat)
CONST_TIFUNC(double, builtins::FIDouble)

CONST_TIFUNC(std::int8_t, builtins::FIInt8)
CONST_TIFUNC(std::int16_t, builtins::FIInt16)
CONST_TIFUNC(std::int32_t, builtins::FIInt32)
CONST_TIFUNC(std::int64_t, builtins::FIInt64)
CONST_TIFUNC(std::uint8_t, builtins::FIUint8)
CONST_TIFUNC(std::uint16_t, builtins::FIUint16)
CONST_TIFUNC(std::uint32_t, builtins::FIUint32)
CONST_TIFUNC(std::uint64_t, builtins::FIUint64)

template <>
TIFUNC(FIConstant<FIFunctionAtom>) {
  auto &func    = t.context.inputs->assem()->funcs().value(m_value);
  auto [tp, cs] = t.instantiate(t.context.getFunc(func));
  return TIResult(w::Subst(), cs, tp);
}

CONST_TIFUNC(FIMessageKeywordAtom, builtins::FIMsgKwT)
CONST_TIFUNC(FIStringAtom, builtins::FIString)

FIOpcode FIMsgValue::opcode() const { return FIOpcode::Msg; }

std::vector<FIRegId> FIMsgValue::deps() const { return m_args; }

TIFUNC(FIMsgValue) {
  auto &msg = t.context.inputs->assem()->msgs().value(m_msg);

  if (msg == builtins::FICurry)
    abort();
  else if (msg == builtins::FICoerce)
    abort();
  else {
    auto a = t.makeVar();

    WTypeStruct::Params params{a};

    for (auto &arg : m_args) params.push_back(t.context.getType(arg));

    auto it = t.context.solverMsgs().find(msg);

    // TODO: This is wrong.  Functions should always take priority over pre-defined messages
    if (it == t.context.solverMsgs().end()) {
      // TODO: Implement diagnostic logging
      // throw std::runtime_error("undeclared message " + msg.name() + "\n" +
      //                          t.state());

      auto params2  = params;
      auto receiver = params2[1];
      params2.erase(params2.begin() + 1); // Remove the receiver

      auto tp = fnew<WAcceptsMessageType>(fnew<WAcceptsMessage>(receiver, msg),
                                          params2);

      return TIResult(params[1]->mgu(tp, t),
                      {fref<WAcceptsMessageConstraint>(
                          a,
                          WAcceptsMessageSet(msg, t.context.solverAccepts()))},
                      a);
    }
    else {
      auto [tp, cs] = t.instantiate(w::sub(t.context.subst, it->second));

      auto su = tp->mgu(fnew<WTypeStruct>(fnew<FIStruct>("fun", params.size()),
                                          params),
                        t);

      return TIResult(su, {}, w::sub(su, a));
    }
  }
}

FIOpcode FITplValue::opcode() const { return FIOpcode::Tpl; }

std::vector<FIRegId> FITplValue::deps() const { return m_values; }

TIFUNC(FITplValue) {
  WTypeStruct::Params params;

  for (auto &value : m_values) params.push_back(t.context.getType(value));

  return TIResult(w::Subst(),
                  w::Constraints(),
                  fnew<WTypeStruct>(fnew<FIStruct>("tpl", params.size()),
                                    params));
}

FIOpcode FIPhiValue::opcode() const { return FIOpcode::Phi; }

std::vector<FIRegId> FIPhiValue::deps() const {
  std::vector<FIRegId> ret;
  for (auto &val : m_values) ret.emplace_back(val.first);
  return ret;
}

TIFUNC(FIPhiValue) {
  fun::FPtr<const w::TypeBase> type = nullptr;
  w::Subst                     subst;
  w::Constraints               constraints;

  for (auto &pair : m_values) {
    auto tp = t.context.getType(pair.first);

    if (!type.nil()) {
      auto su = w::sub(subst, type)->mgu(w::sub(subst, tp), t);
      subst   = composeSubst(su, subst);
    }
    else
      type = w::sub(subst, tp);
  }

  return TIResult(subst, constraints, type);
}

FIOpcode FILdvarValue::opcode() const { return FIOpcode::Ldvar; }

std::vector<FIRegId> FILdvarValue::deps() const { return {}; }

TIFUNC(FILdvarValue) {
  auto it = t.context.env.find(m_var);

  if (it == t.context.env.end())
    // TODO: Implement diagnostic logging
    throw std::runtime_error("unbound variable v" + std::to_string(m_var) +
                             "\n" + t.state());
  else {
    auto [tp, cs] = t.instantiate(w::sub(t.context.subst, it->second));
    return TIResult(w::Subst(), cs, tp);
  }
}

FIOpcode FIStvarValue::opcode() const { return FIOpcode::Stvar; }

std::vector<FIRegId> FIStvarValue::deps() const { return {m_val}; }

TIFUNC(FIStvarValue) {
  auto it = t.context.env.find(m_var);

  if (it == t.context.env.end()) {
    t.context.env[m_var] = w::generalize(w::sub(t.context.subst, t.context.env),
                                         t.context.constraints,
                                         t.context.getType(m_val));
    return TIResult(w::Subst(),
                    w::Constraints(),
                    fnew<WTypeStruct>(builtins::FIVoidT,
                                      WTypeStruct::Params()));
  }
  else {
    auto [tp, cs] = t.instantiate(w::sub(t.context.subst, it->second));
    return TIResult(tp->mgu(t.context.getType(m_val), t),
                    cs,
                    fnew<WTypeStruct>(builtins::FIVoidT,
                                      WTypeStruct::Params()));
  }
}
} // namespace fie
