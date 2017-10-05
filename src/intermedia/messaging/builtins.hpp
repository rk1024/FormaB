#pragma once

#include <vector>

#include "util/ptr.hpp"

#include "message.hpp"

namespace fie {
namespace builtins {
  extern FIMessage FIAdd, FISub, FIMul, FIDiv, FIMod,

      FINeg, FIPos,

      FICeq, FICgt, FIClt,

      FICon, FIDis,

      FIInv,

      FICast,

      FICurry, FICoerce;
}

const std::vector<FIMessage> &fiBuiltinMsgs();
}
