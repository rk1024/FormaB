/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (compiler.cpp)
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

#include "compiler.hpp"

#include "compiler/context.hpp"

namespace pre {
cc::ValueResult FPCompiler::makeValue(cc::BlockCtxPtr    ctx,
                                      const fps::FPExpr *node) const {
  auto _(ctx->pos().move(node));
  return makeValue(ctx.move(), node->infix());
}

cc::ValueResult FPCompiler::makeValue(cc::BlockCtxPtr      ctx,
                                      const fps::FPXInfix *node) const {
  auto _(ctx->pos().move(node));

  switch (node->alt()) {
  case fps::FPXInfix::Add: ctx->errorR("not implemented");
  case fps::FPXInfix::Sub: ctx->errorR("not implemented");
  case fps::FPXInfix::Mul: ctx->errorR("not implemented");
  case fps::FPXInfix::Div: ctx->errorR("not implemented");
  case fps::FPXInfix::Mod: ctx->errorR("not implemented");
  case fps::FPXInfix::Unary: return makeValue(ctx.move(), node->unary());
  }
}

cc::ValueResult FPCompiler::makeValue(cc::BlockCtxPtr      ctx,
                                      const fps::FPXUnary *node) const {
  auto _(ctx->pos().move(node));
  return makeValue(ctx.move(), node->prim());
}

cc::ValueResult FPCompiler::makeValue(cc::BlockCtxPtr     ctx,
                                      const fps::FPXPrim *node) const {
  auto _(ctx->pos().move(node));

  switch (node->alt()) {
  case fps::FPXPrim::Ident: ctx->errorR("not implemented");
  case fps::FPXPrim::Number: return makeNumeric(ctx.move(), node->tok());
  case fps::FPXPrim::Paren: return makeValue(ctx.move(), node->expr());
  }
}

fie::FIGlobalConstant *FPCompiler::compileDAssign(
    const fps::FPDAssign *assign) {

  cc::CompileContext cctx(m_ctx, assign);

  auto ctx = cctx.block();

  auto [ctx2, value] = makeValue(ctx.move(), assign->value());

  return ctx2->globalConstant(assign->name()->value(), value);
}
} // namespace pre
