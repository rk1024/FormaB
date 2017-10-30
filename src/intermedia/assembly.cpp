/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (assembly.cpp)
* Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
*
* FormaB is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* FormaB is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
*
*************************************************************************/

#include "assembly.hpp"

#include "messaging/builtins.hpp"
#include "types/builtins.hpp"

namespace fie {
FIAssembly::FIAssembly() {
  for (auto builtin : fiBuiltinStructs()) m_structs.intern(builtin);
  for (auto builtin : fiBuiltinMsgs()) m_msgs.intern(builtin);
}
}
