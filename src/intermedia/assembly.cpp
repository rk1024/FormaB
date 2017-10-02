#include "assembly.hpp"

#include "types/builtins.hpp"

namespace fie {
FIAssembly::FIAssembly() {
  for (auto builtin : fiBuiltinStructs()) { m_structs.intern(builtin); }
}
}
