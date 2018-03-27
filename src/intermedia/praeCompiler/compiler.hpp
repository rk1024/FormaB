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

#include "intermedia/function/function.hpp"
#include "intermedia/inputs.hpp"
#include "intermedia/messaging/message.hpp"

#include "closures/all.hpp"

#include "ast.hpp"

namespace fie {
// Compiler emit header with BlockClosure, specify node type
#define EMITF_(name, type, ...)                                                \
  emit##name(fun::FLinearPtr<pc::BlockClosure>,                                \
             const frma::FP##type *,                                           \
             ##__VA_ARGS__)

// Compiler emit header with BlockClosure, matching node type
#define EMITF(name, ...) EMITF_(name, name, ##__VA_ARGS__)

// Compiler emitLoad header with BlockClosure, matching node type
#define EMITFL(name, ...)                                                      \
  [[nodiscard]] pc::RegResult EMITF_(Load##name, name, ##__VA_ARGS__)

// Compiler emit header with void return and BlockClosure, specify node type
#define EMITFV_(name, type, ...)                                               \
  [[nodiscard]] pc::VoidResult EMITF_(name, type, ##__VA_ARGS__)

// Compiler emit header with void return and BlockClosure, matching node type
#define EMITFV(name, ...) EMITFV_(name, name, ##__VA_ARGS__)

// Compiler emitStore header with BlockClosure, matching node type
#define EMITFS(name, ...) EMITFV_(Store##name, name, ##__VA_ARGS__)


class FIPraeCompiler : public fun::FObject {
  fun::FPtr<FIInputs> m_inputs;

  EMITFV_(LoadExprsInternal, Exprs, std::vector<FIRegId> &regs);
  EMITFL(Exprs, bool tuple = true);
  EMITFL(Expr);

  EMITFL(LBoolean);
  EMITFL(LNull);
  EMITFL(LNumeric);
  EMITFL(XBlock);
  EMITFL(XControl);
  EMITFL(XFunc);
  EMITFL(XInfix);
  EMITFL(XMember);
  EMITFL(XMsg);
  EMITFL(XParen, bool scope = true);
  EMITFL(XPrim);
  EMITFL(XUnary);

  EMITFS(XMember, const FIRegId &);
  EMITFS(XPrim, const FIRegId &);
  EMITFS(XUnary, const FIRegId &);

  EMITFV(Stmts);
  EMITFV(Stmt);

  EMITFL(SAssign);
  EMITFV(SBind);
  EMITFV(SControl);
  EMITFV(SKeyword);

  EMITFL(AssignValue);
  EMITFV(Bindings, bool mut);
  EMITFV(Binding, bool mut);

  EMITFV(Decl);

  EMITFV(DMsg);
  EMITFV(DType);

public:
  FIPraeCompiler(fun::FPtr<FIInputs>);

  FIFunctionAtom compileEntryPoint(fun::cons_cell<const frma::FPStmts *>);

  FIFunctionAtom compileFunc(fun::cons_cell<const frma::FPXFunc *>);

  std::uint32_t compileType(fun::cons_cell<const frma::FPDType *>);
};

#undef EMITFS
#undef EMITFV
#undef EMITFV_
#undef EMITFL
#undef EMITF
#undef EMITF_
} // namespace fie
