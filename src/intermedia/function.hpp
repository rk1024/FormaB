#pragma once

#include <cstdint>
#include <vector>

#include "opcode.hpp"
#include "util/enableRefCount.hpp"

namespace fie {
class _FIFunction {
  FIBytecode m_body;

public:
  const FIBytecode &body() const { return m_body; }

  _FIFunction(const FIBytecode);
};

using FIFunction  = fun::EnableRefCount<_FIFunction>;
using WFIFunction = fun::EnableWeakRefCount<_FIFunction>;
}
