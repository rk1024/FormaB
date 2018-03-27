/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (flags.hpp)
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

namespace fie {
namespace pc {
#define INITFLAGS(type, base)                                                  \
  namespace type {                                                             \
    enum Flags : base;                                                         \
                                                                               \
    inline constexpr Flags operator|(Flags a, Flags b) {                       \
      return static_cast<Flags>(static_cast<base>(a) | static_cast<base>(b));  \
    }                                                                          \
                                                                               \
    inline constexpr Flags operator&(Flags a, Flags b) {                       \
      return static_cast<Flags>(static_cast<base>(a) & static_cast<base>(b));  \
    }                                                                          \
                                                                               \
    inline constexpr Flags operator~(Flags a) {                                \
      return static_cast<Flags>(~static_cast<base>(a));                        \
    }                                                                          \
  }

  // INITFLAGS(ParenFlags, std::uint16_t);

  // namespace ParenFlags {
  //   enum Flags : std::uint16_t {
  //     Bind  = 1,
  //     // Eval  = 2,
  //     Scope = 4,

  //     Default = Bind /* | Eval */ | Scope,

  //     NoBind  = Default & ~Bind,
  //     // NoEval  = Default & ~Eval,
  //     NoScope = Default & ~Scope,
  //   };
  // }
} // namespace pc
} // namespace fie
