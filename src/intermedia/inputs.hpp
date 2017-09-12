#pragma once

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "assembly.hpp"

namespace fie {
class FIInputs : public fun::FObject {
  fun::FPtr<FIAssembly> m_assem;

public:
  inline fun::FPtr<FIAssembly> assem() { return m_assem; }

  FIInputs(fun::FPtr<FIAssembly>);
};
}
