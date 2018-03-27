/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (gsub.hpp)
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

namespace fun {
static inline std::string gsub(const std::string &src,
                               const std::string &find,
                               const std::string &replace) {
  std::string            dst = src;
  std::string::size_type p   = 0;

  while (true) {
    p = dst.find(find, p);
    if (p == std::string::npos) break;
    dst.replace(p, find.size(), replace);
    p += replace.size();
  }

  return dst;
}
} // namespace fun
