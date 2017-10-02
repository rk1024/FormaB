#pragma once

#include "util/cons.hpp"
#include "util/ptr.hpp"

#include "intermedia/inputs.hpp"

namespace fie {
class FIVerifier : public fun::FObject {
  fun::FPtr<FIInputs> m_inputs;

public:
  FIVerifier(fun::FPtr<FIInputs>);

  void verifyFunc(fun::cons_cell<std::uint32_t>);
};
}
