/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (dumpHex.hpp)
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

#include <functional>
#include <iomanip>
#include <iostream>

namespace fun {
template <typename T>
class _dumpHex {
  T m_num;

public:
  template <typename T2>
  _dumpHex(T2 num) : m_num(static_cast<T>(num)) {}

  template <typename T2>
  friend std::ostream &operator<<(std::ostream &, const _dumpHex<T2> &&);
};

template <typename T>
std::ostream &operator<<(std::ostream &os, const _dumpHex<T> &&dh) {
  auto f = os.flags();
  os << std::hex << std::setfill('0')
     << std::setw(sizeof(T) / sizeof(std::int8_t) * 2);
  os.flags(f);

  constexpr bool
      is_char = std::is_same<std::make_signed<T>, signed char>::value;

  if (is_char)
    os << static_cast<int>(dh.m_num);
  else
    os << dh.m_num;

  return os;
}

template <typename T>
_dumpHex<T> dumpHex(T num) {
  return _dumpHex<T>(num);
}
} // namespace fun
