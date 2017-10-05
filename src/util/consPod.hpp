#pragma once

#include "util/cons.hpp"

#define FUN_CONSPOD(name, ...) struct name : public fun::cons_cell<__VA_ARGS__>

// Mutable getter/setter ("accessor")
#define FCP_ACC(id, name)                                                      \
  inline constexpr auto &name() { return get<id>(); }                          \
  FCP_GET(id, name);

// Immutable getter
#define FCP_GET(id, name)                                                      \
  inline constexpr const auto &name() const { return get<id>(); }

// For use in constructor initializer list
#define FCP_INIT(...) cell_t(fun::cons(__VA_ARGS__))

#define FCP_HASH(name)                                                         \
  namespace std {                                                              \
  template <>                                                                  \
  struct hash<name> {                                                          \
    size_t operator()(const name &var) const {                                 \
      return hash<name::cell_t>{}(var);                                        \
    }                                                                          \
  };                                                                           \
  }
