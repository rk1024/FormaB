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
#include "intermedia/function/value.hpp"

#include "funcContext.hpp"

namespace pre::cc {
class BlockContext;

using BlockCtxPtr = fun::FLinearPtr<BlockContext>;

template <typename T>
using BlkResult = std::pair<BlockCtxPtr, T>;

using RegResult  = BlkResult<fie::FIRegId>;
using VoidResult = BlkResult<void>;

class BlockContext : public fun::FLinearObject<BlockContext> {
  FuncContext * m_ctx;
  fie::FIBlock *m_block;

public:
  constexpr auto &ctx() const { return *m_ctx; }
  constexpr auto &pCtx() const { return m_ctx->ctx(); }
  constexpr auto &fiCtx() const { return m_ctx->fiCtx(); }
  constexpr auto &pos() const { return ctx().pos(); }
  constexpr auto &block() const { return m_block; }

  explicit BlockContext(FuncContext *ctx, fie::FIBlock *block) :
      m_ctx(ctx),
      m_block(block) {}

  void error(const std::string &str) const {
    pCtx().logger().error(pos().curr()->loc(), str);
  }

  [[noreturn]] void errorR(const std::string &str) const {
    pCtx().logger().errorR(pos().curr()->loc(), str);
  }

  void contStatic(fie::FIBlock *next) {
    assert(m_block->cont() == fie::FIBlock::ERR);
    m_block->cont() = fie::FIBlock::Static;
    m_block->next() = next;
  }

  void contBranch(const fie::FIRegId &reg,
                  fie::FIBlock *      next,
                  fie::FIBlock *      Else) {
    assert(m_block->cont() == fie::FIBlock::ERR);
    m_block->cont() = fie::FIBlock::Branch;
    m_block->next() = next;
    m_block->Else() = Else;
    m_block->reg()  = reg;
  }

  void contRet(const fie::FIRegId &reg) {
    assert(m_block->cont() == fie::FIBlock::ERR);
    m_block->cont() = fie::FIBlock::Return;
    m_block->reg()  = reg;
  }

  template <typename... TArgs>
  [[nodiscard]] decltype(auto) newBlock(TArgs &&... args) const {
    return m_ctx->block(std::forward<TArgs>(args)...);
  }

  template <typename T>
  [[nodiscard]] BlkResult<T> ret(const T &val) {
    return std::pair<decltype(fun::wrapLinear(this)), std::decay_t<T>>(
        fun::wrapLinear(this), val);
  }

  template <typename T, typename... TArgs>
  [[nodiscard]] RegResult store(const std::string &name, TArgs &&... args) {
    auto &ins = m_block->body().emplace_back(
        m_ctx->regId(name),
        fiCtx().val<T>(pos().curr()->loc(), std::forward<TArgs>(args)...));
    return RegResult(fun::wrapLinear(this), ins.reg());
  }

  template <typename T, typename... TArgs>
  [[nodiscard]] decltype(auto) msg(TArgs &&... args) {
    return fiCtx().msg<T>(pos().curr()->loc(), std::forward<TArgs>(args)...);
  }
};
} // namespace pre::cc
