#pragma once

#include <cstdint>
#include <string>

namespace fie {
struct FILabel {
  std::uint16_t pos;
  std::string   name;

  FILabel(std::uint16_t _pos, std::string _name) : pos(_pos), name(_name) {}

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
private:
  hash<uint16_t> u2Hash;
  hash<string>   strHash;

public:
  size_t operator()(const fie::FILabel &lbl) const {
    return (u2Hash(lbl.pos) * 2857) ^ strHash(lbl.name);
  }
};
}
