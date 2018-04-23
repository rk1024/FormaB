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

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "parser/ast.hpp"

#include "intermedia/globalConstant.hpp"

#include "context.hpp"

#include "compiler/context.hpp"

namespace pre {
class FPCompiler : public fun::FObject {
  FPContext *m_ctx;

  fie::FIContext &    fiCtx() const { return m_ctx->fiCtx(); }
  const fdi::FLogger &logger() const { return m_ctx->logger(); }

  fie::FIValue *makeNumeric(cc::CompileContext &, const fps::FToken *) const;

  fie::FIValue *makeValue(cc::CompileContext &, const fps::FPExpr *) const;
  fie::FIValue *makeValue(cc::CompileContext &, const fps::FPXInfix *) const;
  fie::FIValue *makeValue(cc::CompileContext &, const fps::FPXUnary *) const;
  fie::FIValue *makeValue(cc::CompileContext &, const fps::FPXPrim *) const;

public:
  FPCompiler(FPContext &ctx) : m_ctx(&ctx) {}

  fie::FIGlobalConstant *compileDAssign(const fps::FPDAssign *);
};
} // namespace pre
