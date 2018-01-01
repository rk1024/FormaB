/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (autoref.hpp)
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

namespace fun {
template <typename>
class FPtr;

template <typename, typename, typename...>
class FMFPtr;

template <typename>
class FWeakPtr;

template <typename T>
struct _autoref {
  using type = const T &;
};

template <>
struct _autoref<void> {
  using type = void;
};

template <typename T>
struct _autoref<T *> {
  using type = T *;
};

template <typename T>
struct _autoref<T &> {
  using type = T &;
};

template <typename T>
struct _autoref<FPtr<T>> {
  using type = FPtr<T>;
};

template <typename T, typename U, typename... TArgs>
struct _autoref<FMFPtr<T, U, TArgs...>> {
  using type = FMFPtr<T, U, TArgs...>;
};

template <typename T>
struct _autoref<FWeakPtr<T>> {
  using type = FWeakPtr<T>;
};

template <typename T>
using autoref = typename _autoref<T>::type;
} // namespace fun
