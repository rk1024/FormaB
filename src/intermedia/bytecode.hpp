/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (bytecode.hpp)
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

#pragma once

#include <cstdint>

#include "util/atom.hpp"

#include "label.hpp"
#include "variable.hpp"

namespace fie {
enum class FIOpcode : std::int8_t {
  Nop = 0, // nop [ -> ]

  Dup, // dup [value -> value, value]
  Pop, // pop [value -> ]
  Ret, // ret [return, $ -> ]

  // Add, // add [a, b -> result]
  // Sub, // sub [a, b -> result]
  // Mul, // mul [a, b -> result]
  // Div, // div [a, b -> result]
  // Mod, // mod [a, b -> result]

  // Neg, // inv [a -> result]
  // Pos, // inv [a -> result]

  // Ceq,  // ceq  [a, b -> result]
  // Cgt,  // cgt  [a, b -> result]
  // Cgtu, // cgtu [a, b -> result]
  // Clt,  // clt  [a, b -> result]
  // Cltu, // cltu [a, b -> result]

  // Con, // con [a, b -> result]
  // Dis, // dis [a, b -> result]

  // Inv, // inv [a -> result]

  Br,  // br  [<addr:i4> | <label:u4>] [ -> ]
  Bez, // bez [<addr:i4> | <label:u4>] [cond -> ]
  Bnz, // bnz [<addr:i4> | <label:u4>] [cond -> ]

  Ldci4, // ldci4 <i4:i4> [ -> i4]
  Ldci8, // ldci8 <i8:i8> [ -> i8]
  Ldcr4, // ldcr4 <r4:r4> [ -> r4]
  Ldcr8, // ldcr8 <r8:r8> [ -> r8]

  Ldnil,  // ldnil [ -> nil]
  Ldvoid, // ldvoid [ -> void]

  Ldvar, // ldvar <var:u4> [ -> var]

  Ldstr, // ldstr <str:u4> [ -> str]
  Ldfun, // ldfun <fun:u4> [ -> fun]
  Ldkw,  // ldkw  <kw:u4> [ -> kw]

  Stvar, // stvar <var:u4> [var -> ]

  Cvi1, // cvi1 [val -> i1]
  Cvi2, // cvi2 [val -> i2]
  Cvi4, // cvi4 [val -> i4]
  Cvi8, // cvi8 [val -> i8]
  Cvu1, // cvu1 [val -> u1]
  Cvu2, // cvu2 [val -> u2]
  Cvu4, // cvu4 [val -> u4]
  Cvu8, // cvu8 [val -> u8]
  Cvr4, // cvr4 [val -> r4]
  Cvr8, // cvr8 [val -> r8]

  Msg, // msg   <msg:u4> [recv, args... -> return]
  // Curry, // curry <msg:u4> [recv, arg -> return]

  Tpl, // tpl  <size:u4> [value[size] -> tuple]
};

struct FIInstruction {
  FIOpcode op;

  union {
    std::int32_t  i4;
    std::uint32_t u4;
    std::int64_t  i8;
    std::uint64_t u8;
    float         r4;
    double        r8;

    struct {
      union {
        std::int32_t  addr;
        std::uint32_t id;
      };

      bool lbl;
    } br;
  };

  FIInstruction(FIOpcode _op) : op(_op) {}
  FIInstruction(FIOpcode _op, std::int32_t _i4) : op(_op), i4(_i4) {}
  FIInstruction(FIOpcode _op, std::uint32_t _u4) : op(_op), u4(_u4) {}
  FIInstruction(FIOpcode _op, std::int64_t _i8) : op(_op), i8(_i8) {}
  FIInstruction(FIOpcode _op, std::uint64_t _u8) : op(_op), u8(_u8) {}
  FIInstruction(FIOpcode _op, float _r4) : op(_op), r4(_r4) {}
  FIInstruction(FIOpcode _op, double _r8) : op(_op), r8(_r8) {}

  static FIInstruction brAddr(FIOpcode op, std::int32_t addr) {
    FIInstruction ret(op);
    ret.br.lbl  = false;
    ret.br.addr = addr;
    return ret;
  }

  static FIInstruction brLbl(FIOpcode op, std::uint32_t id) {
    FIInstruction ret(op);
    ret.br.lbl = true;
    ret.br.id  = id;
    return ret;
  }
};

struct FIBytecode {
  std::vector<FIInstruction> instructions;
  std::vector<FILabel>       labels;
  fun::FAtomStore<FIVariable, std::uint32_t> vars;
};
}
