/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (typeEnv.cpp)
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

#include "typeEnv.hpp"

#include <sstream>

#include "util/gsub.hpp"

#include "scheme.hpp"
#include "type.hpp"

namespace w {
Scheme generalize(const TypeEnv &env, const fun::FPtr<const TypeBase> &t) {
  auto vars = ftv(t);
  for (auto var : ftv(env)) vars.erase(var);
  return Scheme(std::vector<std::string>(vars.begin(), vars.end()), t);
}

std::string printEnv(const TypeEnv &env) {
  std::ostringstream oss;

  for (auto &pair : env)
    oss << "\n  where " + pair.first +
               " :: " + fun::gsub(pair.second.to_string(), "\n", "\n  ");

  return oss.str();
}

std::unordered_set<std::string> Types<TypeEnv>::__ftv(const TypeEnv &e) {
  std::unordered_set<std::string> ret;
  for (auto &pair : e) {
    auto ftv2 = ftv(pair.second);
    ret.insert(ftv2.begin(), ftv2.end());
  }
  return ret;
}

TypeEnv Types<TypeEnv>::__sub(const Subst &subst, const TypeEnv &e) {
  std::unordered_map<std::string, Scheme> retMap;
  for (auto &pair : e) retMap.emplace(pair.first, sub(subst, pair.second));
  return TypeEnv(retMap);
}
} // namespace w
