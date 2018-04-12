/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (token.in.hpp)
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

#include <string>

#include "ast/astBase.hpp"

#pragma astgen friends forward "  "

namespace fps {
class FToken : public FASTBase {
  std::string m_value;

public:
  constexpr auto &value() const { return m_value; }

  FToken(const std::string &value, const fdi::FLocation &loc) :
      FASTBase(loc),
      m_value(value) {}

  FToken(const char *value, const fdi::FLocation &loc) :
      FToken(std::string(value), loc) {}

  virtual void print(std::ostream &os) const override;

#pragma astgen friends "  "
};
} // namespace fps
