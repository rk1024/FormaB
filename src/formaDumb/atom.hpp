#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace frma {
class FAtom {
  static std::unordered_map<std::string, std::intptr_t> s_ids;
  static std::vector<std::string> s_strings;

  std::intptr_t m_id = -1;

  FAtom(std::intptr_t id);

public:
  FAtom() = default;

  static FAtom intern(const std::string &);

  static const std::string &name(const std::intptr_t);

  const std::string &toString() const;

  bool operator==(const FAtom &) const;
  bool operator!=(const FAtom &) const;
  bool operator!() const;

  friend struct std::hash<frma::FAtom>;
};
}

namespace std {
template <>
struct hash<frma::FAtom> {
private:
  hash<std::intptr_t> idHasher;

public:
  std::size_t operator()(const frma::FAtom &atom) const {
    return idHasher(atom.m_id);
  }
};
}
