#include "praeCompiler.hpp"
#include "praeCompilerClosures.hpp"

#include <cassert>
#include <functional>
#include <iostream>
#include <string>

using namespace frma;
using namespace fie::pc;

namespace fie {
enum class NumFormat {
  I1,
  I2,
  I4,
  I8,
  R4,
  R8,
};

static bool isSignNegative(const char *sg) {
  if (sg) {
    switch (*sg) {
    case '-': return true;
    case '+': break;
    default: assert(false);
    }
  }

  return false;
}

template <typename T>
static typename std::enable_if<std::is_integral<T>::value, T>::type
readIntBasic(fun::FPtr<FuncClosure>               closure,
             std::uint8_t                         radix,
             std::int16_t                         expon,
             typename std::make_unsigned<T>::type max,
             typename std::make_unsigned<T>::type negMax,
             const char *                         sg,
             const char *                         d0,
             const char *                         d1) {
  bool neg = isSignNegative(sg);

  if (neg) max = negMax;

  {
    typename std::make_unsigned<T>::type nmax = max / radix, num = 0, next;

    for (; d0 < d1; ++d0) {
      if (num > nmax) goto overflow;

      next = num * radix;

      if (*d0 > 0x5a)
        next += *d0 - 0x61 + 10; // Capital
      else if (*d0 > 0x39)
        next += *d0 - 0x41 + 10; // Lowercase
      else
        next += *d0 - 0x30; // Number

      if (next < num || next > max) goto overflow;

      num = next;
    }

    if (num > max) goto overflow;

    if (expon) {
      assert(expon > 0);

      for (; expon > 0; --expon) {
        if (num > nmax) goto overflow;

        num *= radix;
      }

      if (num > max) goto overflow;
    }

    if (neg) num *= -1;

    return static_cast<T>(num);

  overflow:
    closure->error("integer literal '" + std::string(d0, d1) +
                   "' causes an overflow");
  }
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type readInt(
    fun::FPtr<FuncClosure>               closure,
    std::uint8_t                         radix,
    std::int16_t                         expon,
    bool                                 unsign,
    typename std::make_unsigned<T>::type max,
    const char *                         sg,
    const char *                         d0,
    const char *                         d1) {
  if (!unsign) max >>= 1;

  return readIntBasic<T>(closure, radix, expon, max, max + 1, sg, d0, d1);
}

bool FIPraeCompiler::emitLoadLNumeric(fun::FPtr<FuncClosure> closure,
                                      const FPLNumeric *     numeric) {
  const char *_str = numeric->tok()->value().c_str(), *str = _str, *YYMARKER,
             *sg, *d0, *dp = nullptr, *d1, *es = nullptr, *e0 = nullptr,
             *e1 = nullptr, *tu = nullptr, *tl
      /*!stags:re2c format = ", *@@"; */;

  std::uint8_t radix = 0;
  NumFormat    fmt;

  /*!re2c
    re2c:flags:T = 1;

    re2c:define:YYCTYPE = char;
    re2c:define:YYCURSOR = str;

    re2c:define:YYSTAGN = "@@ = nullptr";

    re2c:yyfill:enable = 0;

    eof = "\x00";

    sign_part = [-+];
    sign = (@sg sign_part)?;

    _hex_digits = [0-9a-fA-F_]+;
    _dec_digits = [0-9_]+;
    _oct_digits = [0-7_]+;
    _bin_digits = [01_]+;

    hex_digits = @d0 _hex_digits @d1;
    dec_digits = @d0 _dec_digits @d1;
    oct_digits = @d0 _oct_digits @d1;
    bin_digits = @d0 _bin_digits @d1;
    float_digits = (@d0 _dec_digits)? @dp "." _dec_digits @d1;

    int_expon = ([eE] (@es "+")? @e0 _dec_digits @e1)?;
    float_expon = ([eE] (@es sign_part)? @e0 _dec_digits @e1)?;

    int_type_spec = (@tu [usUS])? (@tl [yhilYHIL])?;
    dec_type_spec = (@tu [usUS])? (@tl [fdFDyhilYHIL])?;
    float_type_spec = (@tl [fFdD])?;

    hex   = sign "0x"   hex_digits               int_type_spec;
    dec   = sign        dec_digits float_expon   dec_type_spec;
    oct   = sign "0o"   oct_digits               int_type_spec;
    bin   = sign "0b"   bin_digits   int_expon   int_type_spec;
    float = sign      float_digits float_expon float_type_spec;

    * {
      closure->error(
        "invalid numeric literal '" + numeric->tok()->value() + "'"
        " (note: this probably shouldn't happen)");
    }

    hex eof {
      radix = 16;
      goto isInt;
    }

    dec eof {
      radix = 10;
      goto isInt;
    }

    oct eof {
      radix = 8;
      goto isInt;
    }

    bin eof {
      radix = 2;
      goto isInt;
    }

    float eof {
      radix = 10;
      goto isFloat;
    }
  */

  assert(false); // This is here because clang-format

isInt:
  fmt = isSignNegative(es) ? NumFormat::R8 : NumFormat::I4;
  goto emit;
isFloat:
  fmt = NumFormat::R8;
  goto emit;
emit:
  assert(radix);

  if (tl) {
    switch (*tl) {
    case 'f':
    case 'F': fmt = NumFormat::R4; break;
    case 'd':
    case 'D': fmt = NumFormat::R8; break;
    case 'y':
    case 'Y': fmt = NumFormat::I1; break;
    case 'h':
    case 'H': fmt = NumFormat::I2; break;
    case 'i':
    case 'I': fmt = NumFormat::I4; break;
    case 'l':
    case 'L': fmt = NumFormat::I8; break;
    default: assert(false);
    }
  }

  bool unsign = fmt == NumFormat::I1;

  if (tu) {
    switch (*tu) {
    case 'u':
    case 'U': unsign = true; break;
    case 's':
    case 'S': unsign = false; break;
    default: assert(false);
    }
  }

  std::int16_t expon = e0 ? readIntBasic<std::int16_t>(
                                closure, 10u, 0, 0x3ff, 0x3fe, es, e0, e1) :
                            0;

  switch (fmt) {
  case NumFormat::I1:
    closure->emit(
        FIOpcode::Ldci4,
        readInt<std::int32_t>(closure, radix, expon, unsign, 0xff, sg, d0, d1));
    goto convI4;
  case NumFormat::I2:
    closure->emit(FIOpcode::Ldci4,
                  readInt<std::int32_t>(
                      closure, radix, expon, unsign, 0xffff, sg, d0, d1));
    goto convI4;
  case NumFormat::I4:
    closure->emit(FIOpcode::Ldci4,
                  readInt<std::int32_t>(
                      closure, radix, expon, unsign, 0xffffffff, sg, d0, d1));
    goto convI4;
  case NumFormat::I8:
    closure->emit(
        FIOpcode::Ldci4,
        readInt<std::int64_t>(
            closure, radix, expon, unsign, 0xffffffffffffffffl, sg, d0, d1));
    goto convI8;
  case NumFormat::R4: goto emitR4;
  case NumFormat::R8: goto emitR8;
  default: assert(false);
  }
convI4:
  switch (fmt) {
  case NumFormat::I1:
    closure->emit(unsign ? FIOpcode::Cvu1 : FIOpcode::Cvi1);
    break;
  case NumFormat::I2:
    closure->emit(unsign ? FIOpcode::Cvu2 : FIOpcode::Cvi2);
    break;
  case NumFormat::I4:
    if (unsign) closure->emit(FIOpcode::Cvu4);
    break;
  default: assert(false);
  }

  goto done;
convI8:
  switch (fmt) {
  case NumFormat::I8:
    if (unsign) closure->emit(FIOpcode::Cvu8);
    break;
  default: assert(false);
  }
  goto done;
emitR4:
  assert(false);
  goto done;
emitR8:
  assert(false);
  goto done;
done:
  return true;
}
}
