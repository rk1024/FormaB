/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (typeEnv.hpp)
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
#include <unordered_map>

#include "util/gsub.hpp"

#include "scheme.hpp"
#include "types.hpp"

namespace w {
template <typename T>
using TypeEnv = std::unordered_map<T, Scheme>;

template <typename T>
Scheme generalize(const TypeEnv<T> &               env,
                  const fun::FPtr<const TypeBase> &type) {
  auto vars = ftv(type);
  for (auto var : ftv(env)) vars.erase(var);
  return Scheme(std::vector<std::string>(vars.begin(), vars.end()), type);
}

template <typename T>
std::string printEnv(const TypeEnv<T> &env) {
  std::ostringstream oss;

  for (auto &pair : env)
    oss << "\n  where " + pair.first +
               " :: " + fun::gsub(pair.second.to_string(), "\n", "\n  ");

  return oss.str();
}

template <typename T>
struct Types<TypeEnv<T>> {
  static std::unordered_set<std::string> __ftv(const TypeEnv<T> &env) {
    std::unordered_set<std::string> ret;
    for (auto &pair : env) {
      auto ftv2 = ftv(pair.second);
      ret.insert(ftv2.begin(), ftv2.end());
    }
    return ret;
  }

  static TypeEnv<T> __sub(const Subst &subst, const TypeEnv<T> &env) {
    TypeEnv<T> ret;
    for (auto &pair : env) ret.emplace(pair.first, sub(subst, pair.second));
    return ret;
  }
};
} // namespace w
