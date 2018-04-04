/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scheme.cpp)
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

#include "scheme.hpp"

#include "type.hpp"

#include <sstream>

namespace w {
std::string Scheme::to_string() const {
  std::ostringstream oss;

  if (vars().size()) {
    oss << "forall ";

    bool first = true;
    for (auto &var : vars()) {
      if (first)
        first = false;
      else
        oss << ", ";
      oss << var;
    }

    oss << ".";
  }

  oss << type()->to_string();

  if (constraints().size()) {
    oss << " where ";

    bool first = true;
    for (auto &constraint : constraints()) {
      if (first)
        first = false;
      else
        oss << ", ";
      oss << constraint->to_string();
    }
  }

  return oss.str();
}

TypeVars Types<Scheme>::__ftv(const Scheme &s) {
  auto ret = ftv(s.type());
  for (auto &var : s.vars()) ret.erase(var);
  return ret;
}

Scheme Types<Scheme>::__sub(const Subst &subst, const Scheme &s) {
  auto subst2 = subst;
  for (auto &var : s.vars()) subst2.erase(var);
  return Scheme(s.vars(), sub(subst2, s.type()), sub(subst2, s.constraints()));
}
} // namespace w
