/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ti.cpp)
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

#include "ti.hpp"

#include <iostream>
#include <sstream>

#include "util/gsub.hpp"

#include "type.hpp"
#include "typeEnv.hpp"

namespace w {
fun::FPtr<const TypeBase> TIBase::makeVar() {
  std::string name = "@T" + std::to_string(supply);
  TIPos       _(*this, "\e[1mcreate\e[0m " + name);
  ++supply;
  return fnew<TypeVar>(name);
}

fun::FPtr<const TypeBase> TIBase::instantiate(const Scheme &s) {
  TIPos _(*this, "\e[1minstantiate\e[0m " + s.to_string());
  Subst subst;
  for (auto var : s.vars()) subst.emplace(var, makeVar());
  return sub(subst, s.type());
}

void TIBase::debugState() const {
  std::ostringstream oss;

  for (int i = 1; i < stack.size(); ++i) oss << ": ";

  std::cerr << oss.str() << fun::gsub(stack.back(), "\n", "\n" + oss.str())
            << "\n";
}

std::string TIBase::state() const {
  std::ostringstream oss;
  bool               first = true;

  for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
    if (first)
      first = false;
    else
      oss << "\n";
    oss << "  in " << fun::gsub(*it, "\n", "\n  ");
  }

  return oss.str();
}
} // namespace w
