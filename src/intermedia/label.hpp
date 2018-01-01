/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (label.hpp)
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
FUN_CONSPOD(FILabel, std::uint32_t, std::string) {
  FCP_ACC(0, pos);
  FCP_ACC(1, name);

  inline FILabel(std::uint32_t _pos, std::string _name) :
      FCP_INIT(_pos, _name) {}
};
} // namespace fie

FCP_HASH(fie::FILabel);
