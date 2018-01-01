/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (range.hpp)
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

#include <cstdint>

namespace fun {
template <typename T, T...>
struct _tail {};

template <typename T, T car, T... cdr>
struct _tail<T, car, cdr...> {
  static constexpr T value = _tail<T, cdr...>::value;
};

template <typename T, T car>
struct _tail<T, car> {
  static constexpr T value = car;
};

template <typename T, T item>
constexpr T tail();

template <typename T, T... items>
struct int_range {
  using succ = int_range<T, items..., _tail<T, items...>::value + 1>;
};

template <std::size_t... idcs>
using idx_range = int_range<std::size_t, idcs...>;

template <typename T, std::size_t count, T start = 0>
struct _make_int_range {
  using range =
      typename int_range<T, _make_int_range<T, count - 1, start>::range>::succ;
};

template <typename T, T start>
struct _make_int_range<T, 1, start> {
  using range = int_range<T, start>;
};

template <typename T, T start>
struct _make_int_range<T, 0, start> {
  using range = int_range<T>;
};

template <typename T, T count, T start = 0>
using make_int_range = typename _make_int_range<T, count, start>::range;

template <std::size_t count, std::size_t start = 0>
using make_idx_range = make_int_range<std::size_t, count, start>;

template <typename... TItems>
using idx_range_for = make_idx_range<sizeof...(TItems)>;
} // namespace fun
