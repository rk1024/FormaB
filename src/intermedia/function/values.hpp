/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (values.hpp)
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

#include <cstdint>

#include "util/compilerError.hpp"
#include "util/object/object.hpp"

#include "algow/expr.hpp"

#include "ast/astBase.hpp"

#include "intermedia/atoms.hpp"
#include "intermedia/types/builtins.hpp"
#include "regId.hpp"

namespace fie {
class FIBlock;
class FIInstruction;
class TIContext;

enum class FIOpcode {
  Const,
  Nil,
  Void,

  Ldvar,
  Stvar,

  Msg,

  Tpl,
  Phi,
};

enum class FIConstType {
  Bool,

  Float,
  Double,

  Int8,
  Int16,
  Int32,
  Int64,

  Uint8,
  Uint16,
  Uint32,
  Uint64,

  Func,
  MsgKw,
  String,
};

using WTypeStruct = w::Type<FIStruct>;
using TI          = w::TI<TIContext>;

class FIValue : public w::ExprBase<TIContext> {
  const frma::FormaAST *       m_pos;
  fun::FRef<const w::TypeBase> m_type;

public:
  constexpr auto &pos() const { return m_pos; }

  constexpr auto &type() { return m_type; }
  constexpr auto &type() const { return m_type; }

  FIValue(const frma::FormaAST *pos) : m_pos(pos) {}

  virtual FIOpcode opcode() const = 0;

  virtual std::vector<FIRegId> deps() const = 0;

  virtual std::string to_string() const override;
};

class FIOpValue : public FIValue {
  FIOpcode m_opcode;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  FIOpValue(FIOpcode opcode, const frma::FormaAST *pos) :
      FIValue(pos),
      m_opcode(opcode) {}
};

template <typename T>
struct _fi_const_traits;

#define _CONST_TRAITS(_tp, _type)                                              \
  template <>                                                                  \
  struct _fi_const_traits<_tp> {                                               \
    static constexpr FIConstType type = FIConstType::_type;                    \
  };

_CONST_TRAITS(bool, Bool);
_CONST_TRAITS(float, Float);
_CONST_TRAITS(double, Double);

_CONST_TRAITS(std::int8_t, Int8);
_CONST_TRAITS(std::int16_t, Int16);
_CONST_TRAITS(std::int32_t, Int32);
_CONST_TRAITS(std::int64_t, Int64);
_CONST_TRAITS(std::uint8_t, Uint8);
_CONST_TRAITS(std::uint16_t, Uint16);
_CONST_TRAITS(std::uint32_t, Uint32);
_CONST_TRAITS(std::uint64_t, Uint64);

_CONST_TRAITS(FIFunctionAtom, Func);
_CONST_TRAITS(FIMessageKeywordAtom, MsgKw);
_CONST_TRAITS(FIStringAtom, String);

#undef _CONST_TRAITS

class FIConstantBase : public FIValue {
public:
  FIConstantBase(const frma::FormaAST *pos) : FIValue(pos) {}

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  virtual FIConstType constType() const = 0;
};

template <typename T>
class FIConstant : public FIConstantBase {
  T m_value;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  constexpr auto &value() const { return m_value; }

  virtual FIConstType constType() const override {
    return _fi_const_traits<T>::type;
  }

  FIConstant(const T &value, const frma::FormaAST *pos) :
      FIConstantBase(pos),
      m_value(value) {}
  FIConstant(T &&value, const frma::FormaAST *pos) :
      FIConstantBase(pos),
      m_value(value) {}
}; // namespace fie

class FIMsgValue : public FIValue {
  FIMessageAtom        m_msg;
  std::vector<FIRegId> m_args;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  constexpr auto &msg() const { return m_msg; }
  constexpr auto &args() const { return m_args; }

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  FIMsgValue(FIMessageAtom               msg,
             const std::vector<FIRegId> &args,
             const frma::FormaAST *      pos) :
      FIValue(pos),
      m_msg(msg),
      m_args(args) {}
};

class FITplValue : public FIValue {
  std::vector<FIRegId> m_values;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  constexpr auto &values() const { return m_values; }

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  FITplValue(const std::vector<FIRegId> &values, const frma::FormaAST *pos) :
      FIValue(pos),
      m_values(values) {}
};

class FIPhiValue : public FIValue {
  std::vector<std::pair<FIRegId, fun::FWeakPtr<FIBlock>>> m_values;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  constexpr auto &values() const { return m_values; }

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  FIPhiValue(
      const std::vector<std::pair<FIRegId, fun::FWeakPtr<FIBlock>>> &values,
      const frma::FormaAST *                                         pos) :
      FIValue(pos),
      m_values(values) {}
};

class FILdvarValue : public FIValue {
  FIVariableAtom m_var;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  FIVariableAtom var() const { return m_var; }

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  FILdvarValue(FIVariableAtom var, const frma::FormaAST *pos) :
      FIValue(pos),
      m_var(var) {}
};

class FIStvarValue : public FIValue {
  FIVariableAtom m_var;
  FIRegId        m_val;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  FIVariableAtom var() const { return m_var; }
  FIRegId        val() const { return m_val; }

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  FIStvarValue(FIVariableAtom        var,
               const FIRegId &       val,
               const frma::FormaAST *pos) :
      FIValue(pos),
      m_var(var),
      m_val(val) {}
};
} // namespace fie
