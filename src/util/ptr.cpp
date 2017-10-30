/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (ptr.cpp)
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

#include "ptr.hpp"

namespace fun {
#if defined(FPTR_DIAGNOSTIC)
FAtomStore<std::string> __fptr_t_ids;
std::unordered_map<std::size_t, FAtomStore<const void *>> __fptr_ids;
std::unordered_map<std::size_t, FAtomStore<const void *>> __fptr_o_ids;
unsigned int __fptr_indent_lvl;

void __fptr_clr_id(std::ostream &os, std::size_t id, bool big) {
  os << "\e[38;5;" << (((big ? id : id * 8) % 179) + 52) << "m";
}

void __fptr_log_id(std::ostream &os, std::size_t id, bool big) {
  __fptr_clr_id(os, id, big);
  os << std::setw(7) << std::right << std::setfill(' ') << id << "\e[39m";
}
#endif
}
