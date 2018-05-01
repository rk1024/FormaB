/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (const.hpp)
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
class FIConst {
  std::string    m_name;
  FIFunctionBody m_body;

public:
  constexpr auto &name() const { return m_name; }
  constexpr auto &body() { return m_body; }
  constexpr auto &body() const { return m_body; }

  FIConst(const std::string &name, const FIFunctionBody &body) :
      m_name(name),
      m_body(body) {}
};

class FIFoldedConst {
  std::string m_name;
  FIValue *   m_value;

public:
  constexpr auto &name() const { return m_name; }
  constexpr auto &value() const { return m_value; }

  FIFoldedConst(const std::string &name, FIValue *value) :
      m_name(name),
      m_value(value) {}
};
} // namespace fie
