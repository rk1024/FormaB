/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (globalConstant.hpp)
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

#include "function/body.hpp"

namespace fie {
class FIGlobalConstant {
  std::string    m_name;
  FIFunctionBody m_body;

public:
  constexpr auto &name() { return m_name; }
  constexpr auto &name() const { return m_name; }
  constexpr auto &body() { return m_body; }
  constexpr auto &body() const { return m_body; }

  FIGlobalConstant(const std::string &name, const FIFunctionBody &body) :
      m_name(name),
      m_body(body) {}
};
} // namespace fie
