/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (consPod.hpp)
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

#include "util/cons.hpp"

#define FUN_CONSPOD(name, ...) struct name : public fun::cons_cell<__VA_ARGS__>

// Mutable getter/setter ("accessor")
#define FCP_ACC(id, name)                                                      \
  constexpr auto &name() { return this->template get<id>(); }                  \
  FCP_GET(id, name);

// Immutable getter
#define FCP_GET(id, name)                                                      \
  constexpr const auto &name() const { return this->template get<id>(); }

// For use in constructor initializer list
#define FCP_INIT(type, ...) type::cell_t(fun::cons(__VA_ARGS__))

#define FCP_HASH(name)                                                         \
  namespace std {                                                              \
  template <>                                                                  \
  struct hash<name> {                                                          \
    size_t operator()(const name &var) const {                                 \
      return hash<name::cell_t>{}(var);                                        \
    }                                                                          \
  };                                                                           \
  }
