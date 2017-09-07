#pragma once

#include <cstdint>
#include <vector>

#include "bytecode.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

namespace fie {
class FIFunction : public fun::FObject {
  FIBytecode m_body;

public:
  const FIBytecode &body() const { return m_body; }

  FIFunction(const FIBytecode);
};
}
