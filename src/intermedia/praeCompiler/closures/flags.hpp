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

  INITFLAGS(ParenFlags, std::uint16_t);

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
