/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (builtins.cpp)
 * Copyright (C) 2017-2018 Ryan Schroeder, Colin Unger
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
 ************************************************************************/

#include "builtins.hpp"

namespace fie {
namespace builtins {
  fun::FPtr<FIStruct> FIErrorT = fnew<FIStruct>("<error-t>"),

                      FIInt8   = fnew<FIStruct>("sbyte"),
                      FIUint8  = fnew<FIStruct>("byte"),
                      FIInt16  = fnew<FIStruct>("short"),
                      FIUint16 = fnew<FIStruct>("ushort"),
                      FIInt32  = fnew<FIStruct>("int"),
                      FIUint32 = fnew<FIStruct>("uint"),
                      FIInt64  = fnew<FIStruct>("long"),
                      FIUint64 = fnew<FIStruct>("ulong"),

                      FIFloat  = fnew<FIStruct>("float"),
                      FIDouble = fnew<FIStruct>("double"),

                      FIBool = fnew<FIStruct>("bool"),

                      FINilT  = fnew<FIStruct>("<nil-t>"),
                      FIVoidT = fnew<FIStruct>("<void-t>"),

                      FIString = fnew<FIStruct>("string");
}

static std::vector<fun::FPtr<FIStruct>> builtin_vec;

const std::vector<fun::FPtr<FIStruct>> &fiBuiltinStructs() {
  if (builtin_vec.empty()) {
    builtin_vec.push_back(builtins::FIErrorT);

    builtin_vec.push_back(builtins::FIInt8);
    builtin_vec.push_back(builtins::FIUint8);
    builtin_vec.push_back(builtins::FIInt16);
    builtin_vec.push_back(builtins::FIUint16);
    builtin_vec.push_back(builtins::FIInt32);
    builtin_vec.push_back(builtins::FIUint32);
    builtin_vec.push_back(builtins::FIInt64);
    builtin_vec.push_back(builtins::FIUint64);

    builtin_vec.push_back(builtins::FIFloat);
    builtin_vec.push_back(builtins::FIDouble);

    builtin_vec.push_back(builtins::FIBool);

    builtin_vec.push_back(builtins::FINilT);
    builtin_vec.push_back(builtins::FIVoidT);

    builtin_vec.push_back(builtins::FIString);
  }

  return builtin_vec;
}
} // namespace fie
