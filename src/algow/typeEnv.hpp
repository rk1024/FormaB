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

#include <string>
#include <unordered_map>

#include "scheme.hpp"
#include "types.hpp"

namespace w {
using TypeEnv = std::unordered_map<std::string, Scheme>;
Scheme generalize(const TypeEnv &, const fun::FPtr<const TypeBase> &);

std::string printEnv(const TypeEnv &);

template <>
struct Types<TypeEnv> {
  static std::unordered_set<std::string> __ftv(const TypeEnv &);

  static TypeEnv __sub(const Subst &, const TypeEnv &);
};
} // namespace w
