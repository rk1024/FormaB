#pragma once

#include <string.h>

#include <cstdint>
#include <unordered_map>

#include "util/atom.hpp"

#include "bytecode.hpp"
#include "function.hpp"
#include "praeCompilerClosures.hpp"

#include "ast.hpp"

namespace fie {
// Compiler emit header with FuncClosure, specify node type
#define EMITF_(name, type, ...)                                                \
  emit##name(fun::FPtr<pc::FuncClosure>, const frma::FP##type *, ##__VA_ARGS__)

// Compiler emit header with FuncClosure, matching node type
#define EMITF(name, ...) EMITF_(name, name, ##__VA_ARGS__)

// Compiler emitLoad header with FuncClosure, matching node type
#define EMITFL(name, ...) EMITF_(Load##name, name, ##__VA_ARGS__)

// Compiler emitStore header with FuncClosure, matching node type
#define EMITFS(name, ...) EMITF_(Store##name, name, ##__VA_ARGS__)

class FIPraeCompiler : public fun::FObject {
  fun::FAtomStore<fun::FPtr<pc::AssemblyClosure>> m_assems;

  std::uint32_t EMITFL(Exprs, bool tuple = true);
  bool EMITFL(Expr);

  bool EMITFL(LBoolean);
  bool EMITFL(LNumeric);
  bool EMITFL(XBlock);
  bool EMITFL(XControl);
  bool EMITFL(XFunc);
  bool EMITFL(XInfix);
  bool EMITFL(XMember);
  bool EMITFL(XMsg);
  bool EMITFL(XParen, pc::ParenFlags::Flags flags = pc::ParenFlags::Default);
  bool EMITFL(XPrim);
  bool EMITFL(XUnary);

  void EMITFS(XMember);
  void EMITFS(XPrim);
  void EMITFS(XUnary);

  void EMITF(FuncParams);
  void EMITF(FuncParam);

  void EMITF(Stmts);
  void EMITF(Stmt);

  bool EMITFL(SAssign);
  void EMITF(SBind);
  void EMITF(SControl);

  bool EMITFL(AssignValue);
  void EMITF(Bindings, bool mut);
  void EMITF(Binding, bool mut);

public:
  inline auto registerAssembly() {
    auto ret = m_assems.emplace(fnew<pc::AssemblyClosure>());
    return ret;
  }

  std::uint16_t compileEntryPoint(std::size_t, const frma::FPStmts *);

  std::vector<std::pair<const frma::FPBlock *, std::uint16_t>> compileBlocks(
      std::size_t assem, const frma::FPrims *);

  void dump(std::ostream &os) const;
};

#undef EMITFL
#undef EMITF
#undef EMITF_
}
