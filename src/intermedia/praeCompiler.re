#include "praeCompiler.hpp"

#include <cassert>

using namespace frma;

namespace fie {
bool _FIPraeCompiler::emitLoadLNumeric(FuncClosure &     closure,
                                       const FPLNumeric *numeric) {
  switch (numeric->alt()) {
  case FPLNumeric::Hex:
  case FPLNumeric::Dec:
  case FPLNumeric::Oct:
  case FPLNumeric::Bin: closure.emit(FIOpcode::PH_LdInt); return true;
  case FPLNumeric::Float: break;
  default: assert(false);
  }

  closure.emit(FIOpcode::PH_LdReal);

  return true;
}
}
