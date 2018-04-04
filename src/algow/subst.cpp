/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (subst.cpp)
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

#include "subst.hpp"

#include <sstream>

#include "util/gsub.hpp"

#include "type.hpp"
#include "types.hpp"

namespace w {
Subst composeSubst(const Subst &s1, const Subst &s2) {
  Subst ret = s1;
  for (auto &pair : s2) {
#if defined(DEBUG)
    if (auto it = ret.find(pair.first); it != ret.end()) {
      std::cerr << "WARNING in composeSubst: " << pair.first
                << " occurs in both substitutions (" << pair.first << " ~ "
                << it->second->to_string() << " vs. " << pair.first << " ~ "
                << pair.second->to_string() << ")" << std::endl;
    }
#endif

    ret[pair.first] = sub(s1, pair.second);
  }
  return ret;
}

std::string printSubst(const Subst &s) {
  std::ostringstream oss;

  for (auto &pair : s)
    oss << "\n  where " + pair.first + " ~ " +
               fun::gsub(pair.second->to_string(), "\n", "\n  ");

  return oss.str();
}
} // namespace w
