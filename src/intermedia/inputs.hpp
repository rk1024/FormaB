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

public:
  inline fun::FPtr<FIAssembly> assem() { return m_assem; }
  inline decltype(m_funcs) &   funcs() { return m_funcs; }

  FIInputs(fun::FPtr<FIAssembly>);
};
}
