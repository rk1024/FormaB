#pragma once

#include <cstdint>

#include "label.hpp"
#include "util/atom.hpp"

namespace fie {
enum class FIOpcode : std::int8_t {
  Nop = 0, // nop [ -> ]

  Dup, // dup [value -> value, value]
  Pop, // pop [value -> ]
  Ret, // ret [return, $ -> ]

  Add, // add [a, b -> result]
  Sub, // sub [a, b -> result]
  Mul, // mul [a, b -> result]
  Div, // div [a, b -> result]
  Mod, // mod [a, b -> result]

  Neg, // inv [a -> result]
  Pos, // inv [a -> result]

  Ceq,  // ceq  [a, b -> result]
  Cgt,  // cgt  [a, b -> result]
  Cgtu, // cgtu [a, b -> result]
  Clt,  // clt  [a, b -> result]
  Cltu, // cltu [a, b -> result]

  Con, // con [a, b -> result]
  Dis, // dis [a, b -> result]

  Inv, // inv [a -> result]

  Br,  // br  <addr:i2> [ -> ]
  Bez, // bez <addr:i2> [cond -> ]
  Bnz, // bnz <addr:i2> [cond -> ]

  Ldci4, // ldci4 <i4:i4> [ -> i4]
  Ldci8, // ldci8 <i8:i8> [ -> i8]
  Ldcr4, // ldcr4 <r4:i4> [ -> r4]
  Ldcr8, // ldcr8 <r8:i8> [ -> r8]

  Ldnil, // ldnil [ -> nil]

  Ldvar, // ldvar <var:u4> [ -> var]

  Ldstr, // ldstr <atom:u2> [ -> str]
  Ldfun, // ldfun <atom:u2> [ -> fun]

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

  Tpl, // tpl  <size:u4> [value[size] -> tuple]

  PH_Bind,
  PH_LdInt,
  PH_LdReal,
  PH_Msg,
};

struct FIInstruction {
  FIOpcode op;

  union {
    std::int16_t  i2;
    std::uint16_t u2;
    std::int32_t  i4;
    std::uint32_t u4;
    std::int64_t  i8;
    std::uint64_t u8;
    float         r4;
    double        r8;

    struct {
      union {
        std::int16_t  addr;
        std::uint16_t id;
      };

      bool lbl;
    } br;
  };

  FIInstruction(FIOpcode _op) : op(_op) {}
  FIInstruction(FIOpcode _op, std::int16_t _i2) : op(_op), i2(_i2) {}
  FIInstruction(FIOpcode _op, std::uint16_t _u2) : op(_op), u2(_u2) {}
  FIInstruction(FIOpcode _op, std::int32_t _i4) : op(_op), i4(_i4) {}
  FIInstruction(FIOpcode _op, std::uint32_t _u4) : op(_op), u4(_u4) {}
  FIInstruction(FIOpcode _op, std::int64_t _i8) : op(_op), i8(_i8) {}
  FIInstruction(FIOpcode _op, std::uint64_t _u8) : op(_op), u8(_u8) {}
  FIInstruction(FIOpcode _op, float _r4) : op(_op), r4(_r4) {}
  FIInstruction(FIOpcode _op, double _r8) : op(_op), r8(_r8) {}

  static FIInstruction brAddr(FIOpcode op, std::int16_t addr) {
    FIInstruction ret(op);
    ret.br.lbl  = false;
    ret.br.addr = addr;
    return ret;
  }

  static FIInstruction brLbl(FIOpcode op, std::uint16_t id) {
    FIInstruction ret(op);
    ret.br.lbl = true;
    ret.br.id  = id;
    return ret;
  }
};

struct FIBytecode {
  std::vector<FIInstruction> instructions;
  std::vector<FILabel>       labels;
  fun::FAtomStore<std::string, std::uint32_t> vars;
};
}
