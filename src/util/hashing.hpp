/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (hashing.hpp)
* Copyright (C) 2017 Ryan Schroeder, Colin Unger
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

#include <cstdint>
#include <functional>
#include <vector>

namespace fun {
struct forward_hash {
  std::size_t hash;
  forward_hash(std::size_t _hash) : hash(_hash) {}
};

template <typename T>
struct _get_hash {
  inline constexpr std::size_t operator()(T &&val) {
    return std::hash<std::remove_cv_t<std::remove_reference_t<T>>>{}(
        std::forward<T>(val));
  }
};

template <>
struct _get_hash<forward_hash> {
  inline constexpr std::size_t operator()(forward_hash &&fwd) {
    return fwd.hash;
  }
};

template <typename...>
struct combine_hashes;

template <>
struct combine_hashes<> {
  inline void operator()(std::size_t &) const {}
};

template <typename TCar, typename... TCdr>
struct combine_hashes<TCar, TCdr...> {
  void operator()(std::size_t &seed, TCar &&car, TCdr &&... cdr) const {
    seed ^= _get_hash<TCar>{}(std::forward<TCar>(car)) + 0x9e3779b9 +
            (seed << 6) + (seed >> 2);
    combine_hashes<TCdr...>{}(seed, std::forward<TCdr>(cdr)...);
  }
};

template <typename... TArgs>
inline void combineHashes(std::size_t &seed, TArgs &&... args) {
  return combine_hashes<TArgs...>{}(seed, std::forward<TArgs>(args)...);
}

const std::size_t MULTIHASH_EMPTY = 0;

template <typename... TArgs>
std::size_t multiHash(TArgs &&... args) {
  auto ret = MULTIHASH_EMPTY;
  combineHashes(ret, std::forward<TArgs>(args)...);
  return ret;
}

template <typename TIter>
std::size_t iterHash(TIter start, TIter end) {
  auto ret = MULTIHASH_EMPTY;
  for (; start != end; ++start) combineHashes(ret, *start);
  return ret;
}
}
