/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (position.hpp)
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

#include <cstdint>
#include <sstream>
#include <string>

namespace fdi {
class FPosition {
public:
  std::string   filename;
  std::uint32_t line = 1, column = 1;

  FPosition() = default;

  FPosition(const std::string &_filename,
            std::uint32_t      _line,
            std::uint32_t      _column) :
      filename(_filename),
      line(_line),
      column(_column) {}

  void lines(int count = 1) {
    line += count;
    column = 1;
  }

  void columns(int count = 1) { column += count; }

  std::string toString() const {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
  }

  bool operator==(const FPosition &) const;

  bool operator!=(const FPosition &rhs) const { return !operator==(rhs); }

  friend std::ostream &operator<<(std::ostream &, const FPosition &);
};
} // namespace fdi
