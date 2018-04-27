/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (blockContext.hpp)
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

#include "util/linearObject.hpp"
#include "util/linearPtr.hpp"

#include "intermedia/function/block.hpp"
#include "intermedia/value.hpp"

#include "compileContext.hpp"

namespace pre::cc {
class BlockContext;

using BlockCtxPtr = fun::FLinearPtr<BlockContext>;
using ValueResult = std::pair<cc::BlockCtxPtr, fie::FIValue *>;

class BlockContext : public fun::FLinearObject<BlockContext> {
  CompileContext *m_ctx;
  fie::FIBlock *  m_block;

public:
  constexpr auto &ctx() const { return *m_ctx; }
  constexpr auto &pCtx() const { return m_ctx->ctx(); }
  constexpr auto &fiCtx() const { return m_ctx->fiCtx(); }
  constexpr auto &pos() const { return ctx().pos(); }
  constexpr auto &block() const { return m_block; }

  explicit BlockContext(CompileContext *ctx, fie::FIBlock *block) :
      m_ctx(ctx),
      m_block(block) {}

  void error(const std::string &str) const {
    pCtx().logger().error(pos().curr()->loc(), str);
  }

  [[noreturn]] void errorR(const std::string &str) const {
    pCtx().logger().errorR(pos().curr()->loc(), str);
  }

  template <typename T, typename... TArgs>
  [[nodiscard]] ValueResult val(TArgs &&... args) {
    return std::pair(fun::wrapLinear(this),
                     fiCtx().val<T>(pos().curr()->loc(),
                                    std::forward<TArgs>(args)...));
  }

  template <typename... TArgs>
  [[nodiscard]] decltype(auto) globalConstant(TArgs &&... args) {
    return fiCtx().globalConstant(std::forward<TArgs>(args)...);
  }
};
} // namespace pre::cc
