/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (numerics.re)
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

#include "praeforma/compiler.hpp"

namespace pre {
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
static std::enable_if_t<std::is_integral<T>::value, T> readIntBasic(
    cc::BlockCtxPtr &       ctx,
    std::uint8_t            radix,
    std::int16_t            expon,
    std::make_unsigned_t<T> max,
    std::make_unsigned_t<T> negMax,
    const char *            sg,
    const char *            d0,
    const char *            d1) {
  bool neg = isSignNegative(sg);

  if (neg) max = negMax;

  {
    std::make_unsigned_t<T> nmax = max / radix, num = 0, next;

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
    ctx->errorR("integer literal '" + std::string(d0, d1) +
                "' causes an overflow");
  }
}

template <typename T>
std::enable_if_t<std::is_integral<T>::value, T> readInt(
    cc::BlockCtxPtr &       ctx,
    std::uint8_t            radix,
    std::int16_t            expon,
    bool                    unsign,
    std::make_unsigned_t<T> max,
    const char *            sg,
    const char *            d0,
    const char *            d1) {
  if (!unsign) max >>= 1;

  return readIntBasic<T>(ctx.move(), radix, expon, max, max + 1, sg, d0, d1);
}

cc::RegResult FPCompiler::makeNumeric(cc::BlockCtxPtr    ctx,
                                        const fps::FToken *tok) const {
  const char *_str = tok->value().c_str(), *str = _str, *YYMARKER, *sg, *d0,
             *dp = nullptr, *d1, *es = nullptr, *e0 = nullptr, *e1 = nullptr,
             *tu = nullptr, *tl /*!stags:re2c format = ", *@@"; */;

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
      ctx->error("invalid numeric literal '" + tok->value() + "'"
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

isInt:
  // TODO: This seems problematic
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
                                ctx, 10u, 0, 0x3ff, 0x3fe, es, e0, e1)
                          : 0;

  switch (fmt) {
  case NumFormat::I1:
    if (unsign)
      // return closure->emitConst<std::uint8_t>(
      //     "u1",
      //     readInt<std::uint8_t>(
      //         logger(), ctx, radix, expon, true, 0xff, sg, d0, d1));
      ctx->errorR("u1 not implemented");
    else
      // return closure->emitConst<std::int8_t>(
      //     "i1",
      //     readInt<std::int8_t>(
      //         logger(), ctx, radix, expon, false, 0xff, sg, d0, d1));
      ctx->errorR("i1 not implemented");
  case NumFormat::I2:
    if (unsign)
      // return closure->emitConst<std::uint16_t>(
      //     "u2",
      //     readInt<std::uint16_t>(
      //         logger(), ctx, radix, expon, true, 0xffff, sg, d0, d1));
      ctx->errorR("u2 not implemented");
    else
      // return closure->emitConst<std::int16_t>(
      //     "i2",
      //     readInt<std::int16_t>(
      //         logger(), ctx, radix, expon, false, 0xffff, sg, d0, d1));
      ctx->errorR("i2 not implemented");
  case NumFormat::I4:
    if (unsign)
      // return closure->emitConst<std::uint32_t>(
      //     "u4",
      //     readInt<std::uint32_t>(
      //         logger(), ctx, radix, expon, true, 0xffffffff, sg, d0, d1));
      ctx->errorR("u4 not implemented");
    else
      // return closure->emitConst<std::int32_t>(
      //     "i4",
      //     readInt<std::int32_t>(
      //         logger(), ctx, radix, expon, false, 0xffffffff, sg, d0, d1));
      ctx->errorR("i4 not implemented");
  case NumFormat::I8:
    if (unsign)
      // return closure->emitConst<std::uint64_t>(
      //     "u8",
      //     readInt<std::uint64_t>(logger(),
      //                            ctx,
      //                            radix,
      //                            expon,
      //                            true,
      //                            0xffffffffffffffffl,
      //                            sg,
      //                            d0,
      //                            d1));
      ctx->errorR("u8 not implemented");
    else
      // return closure->emitConst<std::int64_t>(
      //     "i8",
      //     readInt<std::int64_t>(logger(),
      //                           ctx,
      //                           radix,
      //                           expon,
      //                           false,
      //                           0xffffffffffffffffl,
      //                           sg,
      //                           d0,
      //                           d1));
      ctx->errorR("i8 not implemented");
  case NumFormat::R4: goto emitR4;
  case NumFormat::R8: goto emitR8;
  default: assert(false);
  }
emitR4:
  // return closure->emitConst<float>("r4", 1337.1337f);
  ctx->errorR("not implemented");
emitR8:
  // TODO: Do this without using stod
  return ctx->store<fie::FIDoubleConstValue>("r8", std::stod(tok->value()));
}
} // namespace pre
