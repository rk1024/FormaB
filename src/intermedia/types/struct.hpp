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

#include <string>
#include <utility>

#include "util/consPod.hpp"
#include "util/object/object.hpp"

namespace fie {
FUN_CONSPOD(FIStruct, std::string, std::uint32_t), public fun::FObject {
  FCP_GET(0, name);
  FCP_GET(1, arity);

  FIStruct(const std::string &_name, std::uint32_t _arity = 0) :
      FCP_INIT(_name, _arity) {}

  constexpr const std::string &to_string() const { return name(); }
};
} // namespace fie

FCP_HASH(fie::FIStruct);
