/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (location.hpp)
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

#include "position.hpp"

namespace fdi {
class FLocation {
public:
  FPosition begin, end;

  FLocation() = default;

  FLocation(const FPosition &_begin, const FPosition &_end) :
      begin(_begin),
      end(_end) {}

  FLocation(const FPosition &pos) : begin(pos), end(pos) {}

  void step() { begin = end; }

  void lines(int count = 1) { end.lines(count); }

  void columns(int count = 1) { end.columns(count); }

  std::string toString() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
  }

  bool operator==(const FLocation &) const;

  bool operator!=(const FLocation &rhs) const { return !operator==(rhs); }

  friend std::ostream &operator<<(std::ostream &, const FLocation &);
};
} // namespace fdi
