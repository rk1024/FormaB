#pragma once

#include <cstdint>
#include <vector>

#include "bytecode.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

namespace fie {
class FIFunction : public fun::FObject {
  std::unordered_map<std::uint32_t, std::uint32_t> m_args;
  FIBytecode m_body;

public:
  const auto &      args() const { return m_args; }
  const FIBytecode &body() const { return m_body; }

  FIFunction(std::unordered_map<std::uint32_t, std::uint32_t>,
             const FIBytecode);
};
}
