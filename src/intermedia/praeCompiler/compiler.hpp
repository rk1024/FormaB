#pragma once

#include <string.h>

#include <cstdint>
#include <unordered_map>

#include "util/atom.hpp"

#include "pipeline/stage.hpp"

#include "intermedia/bytecode.hpp"
#include "intermedia/function.hpp"
#include "intermedia/message.hpp"

#include "closures/all.hpp"

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

class FIPraeCompiler : public fps::FProduces<FIFunction, std::uint32_t>,
                       public fps::FProducesRef<std::string, std::uint32_t>,
                       public fps::FProducesRef<FIMessageId, std::uint32_t> {
  std::uint32_t EMITF_(LoadExprsInternal, Exprs);
  void EMITFL(Exprs, bool tuple = true);
  void EMITFL(Expr);

  void EMITFL(LBoolean);
  void EMITFL(LNull);
  void EMITFL(LNumeric);
  void EMITFL(XBlock);
  void EMITFL(XControl);
  void EMITFL(XFunc);
  void EMITFL(XInfix);
  void EMITFL(XMember);
  void EMITFL(XMsg);
  void EMITFL(XParen, pc::ParenFlags::Flags flags = pc::ParenFlags::Default);
  void EMITFL(XPrim);
  void EMITFL(XUnary);

  void EMITFS(XMember);
  void EMITFS(XPrim);
  void EMITFS(XUnary);

  void EMITF(FuncParams);
  void EMITF(FuncParam);

  void EMITF(Stmts);
  void EMITF(Stmt);

  void EMITFL(SAssign);
  void EMITF(SBind);
  void EMITF(SControl);

  void EMITFL(AssignValue);
  void EMITF(Bindings, bool mut);
  void EMITF(Binding, bool mut);

public:
  std::uint32_t compileEntryPoint(const frma::FPStmts *);

  std::vector<std::pair<const frma::FPBlock *, std::uint32_t>> compileBlocks(
      const frma::FPrims *);
};

#undef EMITFS
#undef EMITFL
#undef EMITF
#undef EMITF_
}
