#pragma once

#include <cstdint>
#include <string>

namespace frma {
class FType {
  std::string  m_name;
  std::int64_t m_uuidA, m_uuidB;

public:
  const std::string &name() const { return m_name; }

  FType(const char *name, std::int64_t uuidA, std::int64_t uuidB);

  bool operator==(const FType &rhs) const;
  bool operator!=(const FType &rhs) const;

  friend struct std::hash<frma::FType>;
};
}

namespace std {
template <>
struct hash<frma::FType> {
private:
  static hash<std::string>  stringhash;
  static hash<std::int64_t> i64hash;

public:
  size_t operator()(const frma::FType &type) {
    size_t ret = stringhash(type.m_name);
    ret        = (ret * 88217) ^ i64hash(type.m_uuidA);
    ret        = (ret * 1259879) ^ i64hash(type.m_uuidB);
    return ret;
  }
};
}
