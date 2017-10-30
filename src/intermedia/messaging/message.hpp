/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (message.hpp)
* Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
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
*************************************************************************/

#pragma once

#include <vector>

#include "util/consPod.hpp"

namespace fie {
FUN_CONSPOD(FIMessage, std::uint32_t, std::string) {
  FCP_GET(0, arity);
  FCP_GET(1, name);

  inline FIMessage(std::uint32_t _arity, std::string _name)
      : FCP_INIT(_arity, _name) {}
};

FUN_CONSPOD(FIMessageKeyword, bool, std::string) {
  FCP_GET(0, arg);
  FCP_GET(1, name);

  inline FIMessageKeyword(bool _arg, std::string _name)
      : FCP_INIT(_arg, _name) {}
};
}

FCP_HASH(fie::FIMessage)
FCP_HASH(fie::FIMessageKeyword)
