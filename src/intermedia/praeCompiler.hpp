#pragma once

#include "util/enableRefCount.hpp"

#include "function.hpp"

#include "ast.hpp"

namespace fie {
class _FIPraeCompiler {
  class AssemblyClosure;
  class FuncClosure;

  void emitLoadExpr(FuncClosure &, const frma::FPExpr *);
  void emitLoadLBoolean(FuncClosure &, const frma::FPLBoolean *);
  void emitLoadLNumeric(FuncClosure &, const frma::FPLNumeric *);
  void emitLoadXCtl(FuncClosure &, const frma::FPXControl *);
  void emitLoadXInfix(FuncClosure &, const frma::FPXInfix *);
  void emitLoadXMember(FuncClosure &, const frma::FPXMember *);
  void emitLoadXPrim(FuncClosure &, const frma::FPXPrim *);
  void emitLoadXUnary(FuncClosure &, const frma::FPXUnary *);
  void emitFunc(FuncClosure &, const frma::FPXFunc *);

public:
  FIFunction compile(const frma::FPXFunc *);
};

using FIPraeCompiler  = fun::EnableRefCount<_FIPraeCompiler>;
using WFIPraeCompiler = fun::EnableWeakRefCount<_FIPraeCompiler>;
}
