#pragma once

#include <cstdint>
#include <string>

#include "util/hashing.hpp"

namespace fie {
struct FILabel {
  std::uint32_t pos;
  std::string   name;

  FILabel(std::uint32_t _pos, std::string _name) : pos(_pos), name(_name) {}

  inline bool operator==(const FILabel &rhs) const {
    return pos == rhs.pos && name == rhs.name;
  }
  inline bool operator!=(const FILabel &rhs) const {
    return !this->operator==(rhs);
  }

  friend struct std::hash<FILabel>;
};
}

namespace std {
template <>
struct hash<fie::FILabel> {
  size_t operator()(const fie::FILabel &lbl) const {
    return fun::multiHash(lbl.pos, lbl.name);
  }
};
}
