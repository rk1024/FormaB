/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (context.hpp)
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

#include <vector>

#include "util/ptrStore.hpp"

#include "diagnostic/logger.hpp"

#include "const.hpp"
#include "function/block.hpp"
#include "function/value.hpp"

namespace fie {
class FIContext {
  fun::FPtrStore<FIValue>       m_values;
  fun::FPtrStore<FIConst>       m_consts;
  fun::FPtrStore<FIFoldedConst> m_foldedConsts;
  fun::FPtrStore<FIBlock>       m_blocks;
  const fdi::FLogger *          m_logger;

public:
  constexpr auto &logger() const { return *m_logger; }

  template <typename T, typename... TArgs>
  [[nodiscard]] decltype(auto) val(TArgs &&... args) {
    return m_values.emplaceP<T>(std::forward<TArgs>(args)...);
  }

  template <typename... TArgs>
  [[nodiscard]] decltype(auto) Const(TArgs &&... args) {
    return m_consts.emplace(std::forward<TArgs>(args)...);
  }

  template <typename... TArgs>
  [[nodiscard]] decltype(auto) foldedConst(TArgs &&... args) {
    return m_foldedConsts.emplace(std::forward<TArgs>(args)...);
  }

  template <typename... TArgs>
  [[nodiscard]] decltype(auto) block(TArgs &&... args) {
    return m_blocks.emplace(std::forward<TArgs>(args)...);
  }

  FIContext(const fdi::FLogger &logger) :
      m_logger(&logger) {}
};
} // namespace fie
