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
fie::FIValue *FPCompiler::makeValue(cc::CompileContext &ctx,
                                    const fps::FPExpr * node) const {
  auto _(ctx.move(node));
  return makeValue(ctx, node->infix());
}

fie::FIValue *FPCompiler::makeValue(cc::CompileContext & ctx,
                                    const fps::FPXInfix *node) const {
  auto _(ctx.move(node));

  switch (node->alt()) {
  case fps::FPXInfix::Add: logger().errorR(ctx.loc(), "not implemented");
  case fps::FPXInfix::Sub: logger().errorR(ctx.loc(), "not implemented");
  case fps::FPXInfix::Mul: logger().errorR(ctx.loc(), "not implemented");
  case fps::FPXInfix::Div: logger().errorR(ctx.loc(), "not implemented");
  case fps::FPXInfix::Mod: logger().errorR(ctx.loc(), "not implemented");
  case fps::FPXInfix::Unary: return makeValue(ctx, node->unary());
  }
}

fie::FIValue *FPCompiler::makeValue(cc::CompileContext & ctx,
                                    const fps::FPXUnary *node) const {
  auto _(ctx.move(node));
  return makeValue(ctx, node->prim());
}

fie::FIValue *FPCompiler::makeValue(cc::CompileContext &ctx,
                                    const fps::FPXPrim *node) const {
  auto _(ctx.move(node));

  switch (node->alt()) {
  case fps::FPXPrim::Ident: logger().errorR(ctx.loc(), "not implemented");
  case fps::FPXPrim::Number: return makeNumeric(ctx, node->tok());
  case fps::FPXPrim::Paren: return makeValue(ctx, node->expr());
  }
}

fie::FIGlobalConstant *FPCompiler::compileDAssign(
    const fps::FPDAssign *assign) {

  cc::CompileContext ctx(assign);

  return fiCtx().globalConstant(assign->name()->value(),
                                makeValue(ctx, assign->value()));
}
} // namespace pre
