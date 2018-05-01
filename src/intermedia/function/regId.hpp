/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (regId.hpp)
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

#include "util/consPod.hpp"

namespace fie {
FUN_CONSPOD(FIRegId, std::uint32_t, std::string) {
  FCP_GET(0, id);
  FCP_GET(1, name);

  FIRegId() = default;

  FIRegId(std::uint32_t _id, const std::string &_name) :
      FCP_INIT(FIRegId, _id, _name) {}
};
} // namespace fie

FCP_HASH(fie::FIRegId);
