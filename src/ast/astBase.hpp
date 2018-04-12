/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (astBase.hpp)
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

#include <sstream>
#include <string>

#include "diagnostic/location.hpp"

namespace fps {
class FASTBase {
protected:
  bool           m_rooted = false;
  fdi::FLocation m_loc;

public:
  constexpr auto &rooted() const { return m_rooted; }
  constexpr auto &loc() const { return m_loc; }

  FASTBase(const fdi::FLocation &loc) : m_loc(loc) {}

  virtual ~FASTBase();

  virtual void print(std::ostream &) const = 0;

  std::string toString() const {
    std::ostringstream oss;
    oss << this;
    return oss.str();
  }

  friend std::ostream &operator<<(std::ostream &, const FASTBase &);
  friend std::ostream &operator<<(std::ostream &, const FASTBase *);
};
} // namespace fps
