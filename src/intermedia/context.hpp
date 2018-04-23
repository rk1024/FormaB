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

#include "diagnostic/logger.hpp"

#include "globalConstant.hpp"
#include "value.hpp"

namespace fie {
class FIContext {
  std::vector<FIValue *>          m_values;
  std::vector<FIGlobalConstant *> m_globalConstants;
  const fdi::FLogger *            m_logger;

public:
  constexpr auto &logger() const { return *m_logger; }

  template <typename T, typename... TArgs>
  T *val(TArgs &&... args) {
    auto *ins = new T(std::forward<TArgs>(args)...);
    m_values.emplace_back(static_cast<FIValue *>(ins));
    return ins;
  }

  template <typename... TArgs>
  auto *globalConstant(TArgs &&... args) {
    auto *konst = new FIGlobalConstant(std::forward<TArgs>(args)...);
    m_globalConstants.emplace_back(konst);
    return konst;
  }

  FIContext(const fdi::FLogger &logger) : m_logger(&logger) {}

  ~FIContext();
};
} // namespace fie
