#pragma once

#include <cstdint>
#include <vector>

#include "util/enableRefCount.hpp"

namespace fie {
enum class FIOpcode : std::int8_t {
  Nop = 0,
  Branch, // branch 00 00
  BrTrue,
  BrFalse,
  LoadLiteral, // format: ... type[1]
  MsgKws,      // format tba
  MsgNil,
  MsgUnary, // format tba
  OpBinary, // format tba
  OpUnary,  // format tba
  Pop,
};

enum class FILiteralType : std::int8_t {
  Boolean,
  Float,
  Int,
  String,
};

class _FIFunction {
  std::vector<std::int8_t> m_body;

public:
  _FIFunction(const std::vector<std::int8_t>);
};

using FIFunction  = fun::EnableRefCount<_FIFunction>;
using WFIFunction = fun::EnableWeakRefCount<_FIFunction>;
}
