#include "assembly.hpp"

#include "messaging/builtins.hpp"
#include "types/builtins.hpp"

namespace fie {
FIAssembly::FIAssembly() {
  for (auto builtin : fiBuiltinStructs()) m_structs.intern(builtin);
  for (auto builtin : fiBuiltinMsgs()) m_msgs.intern(builtin);
}
}
