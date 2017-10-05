#pragma once

#include <vector>

#include "util/ptr.hpp"

#include "struct.hpp"

namespace fie {
namespace builtins {
  extern fun::FPtr<FIStruct> FIErrorT,

      FIInt8, FIUint8, FIInt16, FIUint16, FIInt32, FIUint32, FIInt64, FIUint64,

      FIFloat, FIDouble,

      FIBool,

      FINilT, FIVoidT,

      FIString;
}

const std::vector<fun::FPtr<FIStruct>> &fiBuiltinStructs();
}
