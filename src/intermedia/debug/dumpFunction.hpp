#pragma once

#include <cstdint>
#include <iostream>

#include "util/cons.hpp"
#include "util/object/object.hpp"

#include "intermedia/inputs.hpp"

namespace fie {
class FIDumpFunction : public fun::FObject {
  fun::FPtr<FIInputs> m_inputs;
  std::ostream &      m_os;

public:
  FIDumpFunction(fun::FPtr<FIInputs>, std::ostream &);

  void dumpFunc(fun::cons_cell<std::uint32_t>);
};
}
