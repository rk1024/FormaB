/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (compiler.hpp)
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

#pragma once

#include <string.h>

#include <cstdint>
#include <unordered_map>

#include "util/atom.hpp"

#include "intermedia/bytecode.hpp"
#include "intermedia/function.hpp"
#include "intermedia/inputs.hpp"
#include "intermedia/messaging/message.hpp"

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

class FIPraeCompiler : public fun::FObject {
  fun::FPtr<FIInputs> m_inputs;

  std::uint32_t EMITF_(LoadExprsInternal, Exprs);
  void          EMITFL(Exprs, bool tuple = true);
  void          EMITFL(Expr);

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

  void EMITF(Stmts);
  void EMITF(Stmt);

  void EMITFL(SAssign);
  void EMITF(SBind);
  void EMITF(SControl);
  void EMITF(SKeyword);

  void EMITFL(AssignValue);
  void EMITF(Bindings, bool mut);
  void EMITF(Binding, bool mut);

  void EMITF(Decl);

  void EMITF(DMsg);
  void EMITF(DType);

public:
  FIPraeCompiler(fun::FPtr<FIInputs>);

  std::uint32_t compileEntryPoint(fun::cons_cell<const frma::FPStmts *>);

  std::uint32_t compileFunc(fun::cons_cell<const frma::FPXFunc *>);

  std::uint32_t compileType(fun::cons_cell<const frma::FPDType *>);
};

#undef EMITFS
#undef EMITFL
#undef EMITF
#undef EMITF_
}
