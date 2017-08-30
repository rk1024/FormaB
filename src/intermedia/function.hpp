#pragma once

#include <cstdint>
#include <vector>

#include "opcode.hpp"
#include "util/object.hpp"

namespace fie {
class FIFunction : public fun::FObject {
  FIBytecode m_body;

public:
  const FIBytecode &body() const { return m_body; }

  FIFunction(const FIBytecode);
};
}
