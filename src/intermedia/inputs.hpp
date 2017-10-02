#pragma once

#include <unordered_map>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "ast/astBase.hpp"

#include "assembly.hpp"

namespace fie {
class FIInputs : public fun::FObject {
  fun::FPtr<FIAssembly> m_assem;
  std::unordered_map<const frma::FormaAST *, std::uint32_t> m_funcs;
  std::unordered_map<const frma::FormaAST *, std::uint32_t> m_structs;

public:
  inline fun::FPtr<FIAssembly> assem() { return m_assem; }
  inline auto &                funcs() { return m_funcs; }
  inline auto &                structs() { return m_structs; }

  FIInputs(fun::FPtr<FIAssembly>);
};
}
