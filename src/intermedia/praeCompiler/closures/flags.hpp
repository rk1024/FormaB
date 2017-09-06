#pragma once

#include <cstdint>

namespace fie {
namespace pc {
#define INITFLAGS(type)                                                        \
  namespace type {                                                             \
    enum Flags : std::uint16_t;                                                \
                                                                               \
    inline constexpr Flags operator|(Flags a, Flags b) {                       \
      return static_cast<Flags>(static_cast<std::uint16_t>(a) |                \
                                static_cast<std::uint16_t>(b));                \
    }                                                                          \
                                                                               \
    inline constexpr Flags operator&(Flags a, Flags b) {                       \
      return static_cast<Flags>(static_cast<std::uint16_t>(a) &                \
                                static_cast<std::uint16_t>(b));                \
    }                                                                          \
                                                                               \
    inline constexpr Flags operator~(Flags a) {                                \
      return static_cast<Flags>(~static_cast<std::uint16_t>(a));               \
    }                                                                          \
  }

  INITFLAGS(ParenFlags)

  namespace ParenFlags {
    enum Flags : std::uint16_t {
      Bind  = 1,
      Eval  = 2,
      Scope = 4,

      Default = Bind | Eval | Scope,

      NoBind  = Default & ~Bind,
      NoEval  = Default & ~Eval,
      NoScope = Default & ~Scope,
    };
  }
}
}
