/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (instructions.hpp)
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

#include "intermedia/atoms.hpp"
#include "intermedia/types/builtins.hpp"

namespace fie {
enum class FIOpcode : std::int8_t {
  Nop = 0, // nop [ -> ]

  Dup, // dup [value -> value, value]
  Pop, // pop [value -> ]
  Ret, // ret [return, $ -> ]

  Br,  // br  [<addr:i4> | <label:u4>] [ -> ]
  Bez, // bez [<addr:i4> | <label:u4>] [cond -> ]
  Bnz, // bnz [<addr:i4> | <label:u4>] [cond -> ]

  Load, // load [type] [value] [ -> value]

  Ldnil,  // ldnil [ -> nil]
  Ldvoid, // ldvoid [ -> void]

  Ldvar, // ldvar <var:u4> [ -> var]

  Stvar, // stvar <var:u4> [var -> ]

  Msg, // msg <msg:u4> [recv, args... -> return]

  Tpl, // tpl <size:u4> [value[size] -> tuple]
};

class FIInstructionBase : public fun::FObject {
public:
  virtual FIOpcode opcode() const = 0;
};

class FIBasicInstruction : public FIInstructionBase {
  FIOpcode m_opcode;

public:
  virtual FIOpcode opcode() const override;

  FIBasicInstruction(FIOpcode opcode) : m_opcode(opcode) {}
};

class FIBranchInstruction : public FIBasicInstruction {
  FILabelAtom m_label;

public:
  FILabelAtom label() const { return m_label; }

  FIBranchInstruction(FIOpcode opcode, FILabelAtom label) :
      FIBasicInstruction(opcode),
      m_label(label) {
    switch (opcode) {
    case FIOpcode::Br:
    case FIOpcode::Bez:
    case FIOpcode::Bnz: break;
    default: abort();
    }
  }
};

template <typename T>
struct _fi_load_traits;

#define _LOAD_TRAITS(_tp, _struct)                                             \
  template <>                                                                  \
  struct _fi_load_traits<_tp> {                                                \
    static constexpr fun::FPtr<FIStruct> *type = &_struct;                     \
  };

_LOAD_TRAITS(bool, builtins::FIBool);
_LOAD_TRAITS(float, builtins::FIFloat);
_LOAD_TRAITS(double, builtins::FIDouble);

_LOAD_TRAITS(std::int8_t, builtins::FIInt8);
_LOAD_TRAITS(std::int16_t, builtins::FIInt16);
_LOAD_TRAITS(std::int32_t, builtins::FIInt32);
_LOAD_TRAITS(std::int64_t, builtins::FIInt64);
_LOAD_TRAITS(std::uint8_t, builtins::FIUint8);
_LOAD_TRAITS(std::uint16_t, builtins::FIUint16);
_LOAD_TRAITS(std::uint32_t, builtins::FIUint32);
_LOAD_TRAITS(std::uint64_t, builtins::FIUint64);

_LOAD_TRAITS(FIFunctionAtom, builtins::FIErrorT);       // TODO
_LOAD_TRAITS(FIMessageKeywordAtom, builtins::FIErrorT); // TODO
_LOAD_TRAITS(FIStringAtom, builtins::FIString);

#undef _LOAD_TRAITS

class FILoadInstructionBase : public FIInstructionBase {
public:
  virtual FIOpcode opcode() const override;

  virtual fun::FPtr<FIStruct> &type() const = 0;
};

template <typename T>
class FILoadInstruction : public FILoadInstructionBase {
  T m_value;

public:
  const T &value() const { return m_value; }

  virtual fun::FPtr<FIStruct> &type() const override {
    return *_fi_load_traits<T>::type;
  }

  FILoadInstruction(const T &value) : m_value(value) {}
  FILoadInstruction(T &&value) : m_value(value) {}
};

class FIMessageInstruction : public FIInstructionBase {
  FIMessageAtom m_msg;

public:
  FIMessageAtom msg() const { return m_msg; }

  virtual FIOpcode opcode() const override;

  FIMessageInstruction(FIMessageAtom msg) : m_msg(msg) {}
};

class FITupleInstruction : public FIInstructionBase {
  std::uint32_t m_count;

public:
  std::uint32_t count() const { return m_count; }

  virtual FIOpcode opcode() const override;

  FITupleInstruction(std::uint32_t count) : m_count(count) {}
};

class FIVarInstruction : public FIBasicInstruction {
  FIVariableAtom m_var;

public:
  FIVariableAtom var() const { return m_var; }

  FIVarInstruction(FIOpcode opcode, FIVariableAtom var) :
      FIBasicInstruction(opcode),
      m_var(var) {
    switch (opcode) {
    case FIOpcode::Ldvar:
    case FIOpcode::Stvar: break;
    default: abort();
    }
  }
};
} // namespace fie
