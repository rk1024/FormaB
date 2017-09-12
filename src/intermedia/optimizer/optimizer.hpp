#pragma once

#include "util/cons.hpp"
#include "util/object/object.hpp"

#include "intermedia/function.hpp"

namespace fie {
class FIOptimizer : public fun::FObject {

public:
  void optimizeFunc(fun::cons_cell<std::uint32_t>);
};
}
