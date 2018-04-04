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
class FIInstruction;
class TIContext;

enum class FIOpcode : std::int8_t {
  Const,
  Nil,
  Void,
  Ldvar,
  Stvar,
  Msg,
  Tpl,
  Phi,
};

using WTypeStruct = w::Type<FIStruct>;
using TI          = w::TI<TIContext>;

class FIValue : public w::ExprBase<TIContext> {
  const frma::FormaAST *m_pos;

public:
  constexpr auto &pos() const { return m_pos; }

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

#define _CONST_TRAITS(_tp, _struct)                                            \
  template <>                                                                  \
  struct _fi_const_traits<_tp> {                                               \
    static constexpr fun::FPtr<FIStruct> *type = &_struct;                     \
  };

_CONST_TRAITS(bool, builtins::FIBool);
_CONST_TRAITS(float, builtins::FIFloat);
_CONST_TRAITS(double, builtins::FIDouble);

_CONST_TRAITS(std::int8_t, builtins::FIInt8);
_CONST_TRAITS(std::int16_t, builtins::FIInt16);
_CONST_TRAITS(std::int32_t, builtins::FIInt32);
_CONST_TRAITS(std::int64_t, builtins::FIInt64);
_CONST_TRAITS(std::uint8_t, builtins::FIUint8);
_CONST_TRAITS(std::uint16_t, builtins::FIUint16);
_CONST_TRAITS(std::uint32_t, builtins::FIUint32);
_CONST_TRAITS(std::uint64_t, builtins::FIUint64);

_CONST_TRAITS(FIFunctionAtom, builtins::FIFuncT);
_CONST_TRAITS(FIMessageKeywordAtom, builtins::FIMsgKeywordT);
_CONST_TRAITS(FIStringAtom, builtins::FIString);

#undef _CONST_TRAITS

class FIConstantBase : public FIValue {
public:
  FIConstantBase(const frma::FormaAST *pos) : FIValue(pos) {}

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  virtual fun::FPtr<FIStruct> &type() const = 0;
};

template <typename T>
class FIConstant : public FIConstantBase {
  T m_value;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  constexpr auto &value() const { return m_value; }

  virtual fun::FPtr<FIStruct> &type() const override {
    return *_fi_const_traits<T>::type;
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
  std::vector<FIRegId> m_values;

protected:
  virtual TIResult tiImpl(TI &) const override;

public:
  constexpr auto &values() const { return m_values; }

  virtual FIOpcode opcode() const override;

  virtual std::vector<FIRegId> deps() const override;

  FIPhiValue(const std::vector<FIRegId> &values, const frma::FormaAST *pos) :
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
