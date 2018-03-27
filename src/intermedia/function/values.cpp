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

#define TIFUNC(type) type::TIResult type::tiImpl([[maybe_unused]] TI &t) const

FIOpcode FIOpValue::opcode() const { return m_opcode; }

std::vector<FIRegId> FIOpValue::deps() const { return {}; }

TIFUNC(FIOpValue) {
  switch (m_opcode) {
  case FIOpcode::Nil:
    return TIResult(w::Subst(),
                    fnew<WTypeStruct>(builtins::FIErrorT,
                                      WTypeStruct::Params()));
  case FIOpcode::Void:
    return TIResult(w::Subst(),
                    fnew<WTypeStruct>(builtins::FIVoidT,
                                      WTypeStruct::Params()));
  default: abort();
  }
}

std::string FIOpValue::to_string() const { return opcodeName(m_opcode); }

FIOpcode FIConstantBase::opcode() const { return FIOpcode::Const; }

std::vector<FIRegId> FIConstantBase::deps() const { return {}; }

#define CONST_TIFUNC(_tp)                                                      \
  template <>                                                                  \
  TIFUNC(FIConstant<_tp>) {                                                    \
    return TIResult(w::Subst(),                                                \
                    fnew<WTypeStruct>(*_fi_const_traits<_tp>::type,            \
                                      WTypeStruct::Params()));                 \
  }

CONST_TIFUNC(bool)
CONST_TIFUNC(float)
CONST_TIFUNC(double)

CONST_TIFUNC(std::int8_t)
CONST_TIFUNC(std::int16_t)
CONST_TIFUNC(std::int32_t)
CONST_TIFUNC(std::int64_t)
CONST_TIFUNC(std::uint8_t)
CONST_TIFUNC(std::uint16_t)
CONST_TIFUNC(std::uint32_t)
CONST_TIFUNC(std::uint64_t)

template <>
TIFUNC(FIConstant<FIFunctionAtom>) {
  auto &func = t.context.inputs->assem()->funcs().value(m_value);
  return TIResult(w::Subst(), t.instantiate(t.context.solverFuncs().at(func)));
}

CONST_TIFUNC(FIMessageKeywordAtom)
CONST_TIFUNC(FIStringAtom)

FIOpcode FIMsgValue::opcode() const { return FIOpcode::Msg; }

std::vector<FIRegId> FIMsgValue::deps() const { return m_args; }

TIFUNC(FIMsgValue) {
  return TIResult(w::Subst(),
                  fnew<WTypeStruct>(builtins::FIVoidT,
                                    WTypeStruct::Params())); // TODO
}

std::string FIMsgValue::to_string() const {
  std::ostringstream oss;
  oss << "msg m" << m_msg;
  for (auto &value : m_args) oss << " " << value.id();
  return oss.str();
}

FIOpcode FITplValue::opcode() const { return FIOpcode::Tpl; }

std::vector<FIRegId> FITplValue::deps() const { return m_values; }

TIFUNC(FITplValue) { return TIResult(w::Subst(), nullptr); }

std::string FITplValue::to_string() const {
  std::ostringstream oss;
  oss << "tpl";
  for (auto &value : m_values) oss << " " << value.id();
  return oss.str();
}

FIOpcode FIPhiValue::opcode() const { return FIOpcode::Phi; }

std::vector<FIRegId> FIPhiValue::deps() const { return m_values; }

TIFUNC(FIPhiValue) {
  fun::FPtr<const w::TypeBase> type = nullptr;
  w::Subst                     subst;

  for (auto &reg : m_values) {
    auto tp = t.context.getType(reg);

    if (!type.nil()) {
      auto su = w::sub(subst, type)->mgu(w::sub(subst, tp), t);
      subst   = composeSubst(su, subst);
    }
    else
      type = w::sub(subst, tp);
  }

  return TIResult(subst, type);
}

std::string FIPhiValue::to_string() const {
  std::ostringstream oss;
  oss << "phi";
  for (auto &value : m_values) oss << " " << value.id();
  return oss.str();
}

FIOpcode FILdvarValue::opcode() const { return FIOpcode::Ldvar; }

std::vector<FIRegId> FILdvarValue::deps() const { return {}; }

TIFUNC(FILdvarValue) {
  auto it = t.context.env.find(m_var);

  if (it == t.context.env.end())
    // TODO: Implement diagnostic logging
    throw std::runtime_error("unbound variable v" + std::to_string(m_var) +
                             "\n" + t.state());
  else
    return TIResult(w::Subst(), t.instantiate(it->second));
}

std::string FILdvarValue::to_string() const {
  return "ldvar v" + std::to_string(m_var);
}

FIOpcode FIStvarValue::opcode() const { return FIOpcode::Stvar; }

std::vector<FIRegId> FIStvarValue::deps() const { return {m_val}; }

TIFUNC(FIStvarValue) {
  auto it = t.context.env.find(m_var);

  if (it == t.context.env.end()) {
    t.context.env[m_var] = w::generalize(w::sub(t.context.subst, t.context.env),
                                         t.context.getType(m_val));
    return TIResult(w::Subst(),
                    fnew<WTypeStruct>(builtins::FIVoidT,
                                      WTypeStruct::Params()));
  }
  else {
    return TIResult(t.instantiate(it->second)->mgu(t.context.getType(m_val), t),
                    fnew<WTypeStruct>(builtins::FIVoidT,
                                      WTypeStruct::Params()));
  }
}

std::string FIStvarValue::to_string() const {
  return "stvar v" + std::to_string(m_var) + " <- " +
         std::to_string(m_val.id());
}
} // namespace fie
