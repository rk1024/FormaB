/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (struct.hpp)
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
#include <utility>

#include "util/consPod.hpp"
#include "util/object/object.hpp"

namespace fie {
FUN_CONSPOD(FIStruct, std::string, std::uint32_t), public fun::FObject {
  FCP_GET(0, name);
  FCP_GET(1, arity);

  FIStruct(const std::string &_name, std::uint32_t _arity = 0) :
      FCP_INIT(FIStruct, _name, _arity) {}

  constexpr const std::string &to_string() const { return name(); }
};

template <typename...>
struct _paramstruct_join;

template <typename T>
struct _paramstruct_join<T> {
  static void run(std::ostringstream &oss, const fun::cons_cell<T> &cell) {
    oss << cell.car.to_string();
  }
};

template <typename TCar, typename... TCdr>
struct _paramstruct_join<TCar, TCdr...> {
  static void run(std::ostringstream &                 oss,
                  const fun::cons_cell<TCar, TCdr...> &cell) {
    oss << cell.car.to_string() << ", ";
    _paramstruct_join<TCdr...>::run(oss, cell.cdr);
  };
};

template <typename... TArgs>
FUN_CONSPOD(FIParamStruct,
            std::string,
            fun::cons_cell<TArgs...>,
            std::uint32_t),
    public fun::FObject {
  FCP_GET(0, name);
  FCP_GET(1, params);
  FCP_GET(2, arity);

  FIParamStruct(const std::string &             _name,
                const fun::cons_cell<TArgs...> &_params,
                std::uint32_t                   _arity = 0) :
      FCP_INIT(FIParamStruct, _name, _params, _arity) {}

  const std::string to_string() const {
    std::ostringstream oss;
    oss << name() << "(";

    _paramstruct_join<TArgs...>::run(oss, params());

    oss << ")";

    return oss.str();
  }
};
} // namespace fie

FCP_HASH(fie::FIStruct);

// TODO: Might wanna make a macro for this...
namespace std {
template <typename... TArgs>
struct hash<fie::FIParamStruct<TArgs...>> {
  size_t operator()(const fie::FIParamStruct<TArgs...> &var) const {
    return hash<typename fie::FIParamStruct<TArgs...>::cell_t>{}(var);
  }
};
} // namespace std
