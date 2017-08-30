#pragma once

#include <string.h>

#include <cstdint>
#include <unordered_map>

#include "util/atom.hpp"

#include "function.hpp"
#include "opcode.hpp"
#include "praeCompilerClosures.hpp"

#include "ast.hpp"

namespace fie {
class FIPraeCompiler : public fun::FObject {
  fun::FAtomStore<fun::FPtr<pc::AssemblyClosure>> m_assems;

  std::uint32_t emitLoadExprs(fun::FPtr<pc::_FuncClosure>,
                              const frma::FPExprs *,
                              bool = true);
  bool emitLoadExpr(fun::FPtr<pc::_FuncClosure>, const frma::FPExpr *);

  bool emitLoadLBoolean(fun::FPtr<pc::_FuncClosure>, const frma::FPLBoolean *);
  bool emitLoadLNumeric(fun::FPtr<pc::_FuncClosure>, const frma::FPLNumeric *);
  bool emitLoadXBlock(fun::FPtr<pc::_FuncClosure>, const frma::FPXBlock *);
  bool emitLoadXControl(fun::FPtr<pc::_FuncClosure>, const frma::FPXControl *);
  bool emitLoadXFunc(fun::FPtr<pc::_FuncClosure>, const frma::FPXFunc *);
  bool emitLoadXInfix(fun::FPtr<pc::_FuncClosure>, const frma::FPXInfix *);
  bool emitLoadXMember(fun::FPtr<pc::_FuncClosure>, const frma::FPXMember *);
  bool emitLoadXParen(fun::FPtr<pc::_FuncClosure>,
                      const frma::FPXParen *,
                      bool = false);
  bool emitLoadXPrim(fun::FPtr<pc::_FuncClosure>, const frma::FPXPrim *);
  bool emitLoadXUnary(fun::FPtr<pc::_FuncClosure>, const frma::FPXUnary *);

  void emitFuncParams(fun::FPtr<pc::_FuncClosure>, const frma::FPFuncParams *);
  void emitFuncParam(fun::FPtr<pc::_FuncClosure>, const frma::FPFuncParam *);

  void emitStmts(fun::FPtr<pc::_FuncClosure>, const frma::FPStmts *);
  void emitStmt(fun::FPtr<pc::_FuncClosure>, const frma::FPStmt *);

  void emitSBind(fun::FPtr<pc::_FuncClosure>, const frma::FPSBind *);
  void emitSControl(fun::FPtr<pc::_FuncClosure>, const frma::FPSControl *);

  void emitBindings(fun::FPtr<pc::_FuncClosure>,
                    const frma::FPBindings *,
                    bool);
  void emitBinding(fun::FPtr<pc::_FuncClosure>, const frma::FPBinding *, bool);

public:
  inline auto registerAssembly() {
    return m_assems.emplace(fnew<pc::AssemblyClosure>());
  }

  std::uint16_t compileEntryPoint(decltype(m_assems.emplace()),
                                  const frma::FPStmts *);

  std::vector<std::pair<const frma::FPBlock *, std::uint16_t>> compileBlocks(
      std::size_t assem, const frma::FPrims *);

  void dump(std::ostream &os) const;
};
}
