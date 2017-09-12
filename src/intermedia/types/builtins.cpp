#include "builtins.hpp"

namespace fie {
namespace builtins {
  fun::FPtr<FIStruct> FIErrorT = fnew<FIStruct>("<error-t>");

  fun::FPtr<FIStruct> FIInt8   = fnew<FIStruct>("sbyte");
  fun::FPtr<FIStruct> FIUint8  = fnew<FIStruct>("byte");
  fun::FPtr<FIStruct> FIInt16  = fnew<FIStruct>("short");
  fun::FPtr<FIStruct> FIUint16 = fnew<FIStruct>("ushort");
  fun::FPtr<FIStruct> FIInt32  = fnew<FIStruct>("int");
  fun::FPtr<FIStruct> FIUint32 = fnew<FIStruct>("uint");
  fun::FPtr<FIStruct> FIInt64  = fnew<FIStruct>("long");
  fun::FPtr<FIStruct> FIUint64 = fnew<FIStruct>("ulong");

  fun::FPtr<FIStruct> FIFloat  = fnew<FIStruct>("float");
  fun::FPtr<FIStruct> FIDouble = fnew<FIStruct>("double");

  fun::FPtr<FIStruct> FIBool = fnew<FIStruct>("bool");

  fun::FPtr<FIStruct> FINil  = fnew<FIStruct>("<nil-t>");
  fun::FPtr<FIStruct> FIVoid = fnew<FIStruct>("<void-t>");

  fun::FPtr<FIStruct> FIString = fnew<FIStruct>("string");
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

    builtin_vec.push_back(builtins::FINil);
    builtin_vec.push_back(builtins::FIVoid);

    builtin_vec.push_back(builtins::FIString);
  }

  return builtin_vec;
}
}
