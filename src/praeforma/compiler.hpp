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

#include "intermedia/const.hpp"

#include "context.hpp"

#include "compiler/context.hpp"

namespace pre {
class FPCompiler : public fun::FObject {
  FPContext *m_ctx; // TODO: Maybe don't make this a field?

  cc::RegResult makeNumeric(cc::BlockCtxPtr, const fps::FToken *) const;

  cc::RegResult emitStore(cc::BlockCtxPtr, const fps::FPExpr *) const;
  cc::RegResult emitStore(cc::BlockCtxPtr, const fps::FPXInfix *) const;
  cc::RegResult emitStore(cc::BlockCtxPtr, const fps::FPXUnary *) const;
  cc::RegResult emitStore(cc::BlockCtxPtr, const fps::FPXPrim *) const;
  cc::RegResult emitStore(cc::BlockCtxPtr, const fps::FPXParen *) const;
  cc::RegResult emitStore(cc::BlockCtxPtr, const fps::FPXControl *) const;

  template <std::size_t n, typename T, typename... TArgs>
  cc::BlockCtxPtr emitStores_impl(std::array<fie::FIRegId, n> &ret,
                                  cc::BlockCtxPtr              ctx,
                                  std::size_t                  i,
                                  T &&                         first,
                                  TArgs &&... args) const {
    auto [ctx2, val] = emitStore(ctx.move(), first);
    ret[i]           = val;
    return emitStores_impl(ret, ctx2.move(), ++i, std::forward<TArgs>(args)...);
  }

  template <std::size_t n>
  cc::BlockCtxPtr emitStores_impl(std::array<fie::FIRegId, n> &,
                                  cc::BlockCtxPtr ctx,
                                  std::size_t) const {
    return ctx.move();
  }

  template <typename... TArgs>
  cc::BlkResult<std::array<fie::FIRegId, sizeof...(TArgs)>> makeValues(
      cc::BlockCtxPtr ctx, TArgs &&... args) const {
    std::array<fie::FIRegId, sizeof...(TArgs)> ret;

    auto ctx2 = emitStores_impl(ret,
                                ctx.move(),
                                0,
                                std::forward<TArgs>(args)...);

    return ctx2->ret(ret);
  }

  template <typename... TArgs>
  cc::RegResult makeMsg(cc::BlockCtxPtr    ctx,
                        const std::string &name,
                        const std::string &msg,
                        TArgs &&... args) const {
    auto [ctx2, params] = makeValues(ctx.move(), std::forward<TArgs>(args)...);

    return ctx2->template store<fie::FIMsgValue>(
        name, msg, std::vector<fie::FIRegId>(params.begin(), params.end()));
  }

public:
  FPCompiler(FPContext &ctx) : m_ctx(&ctx) {}

  fie::FIConst *compileDAssign(const fps::FPDAssign *);
};
} // namespace pre
