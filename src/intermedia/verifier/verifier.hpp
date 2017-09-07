#pragma once

#include "util/ptr.hpp"

#include "intermedia/assembly.hpp"

namespace fie {
class FIVerifier {
  fun::FPtr<FIAssembly> m_assem;

public:
  FIVerifier(fun::FPtr<FIAssembly>);
};
}
