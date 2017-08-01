#include "type.hpp"

namespace frma {
FType::FType(const char *name, std::int64_t uuidA, std::int64_t uuidB)
    : m_name(name), m_uuidA(uuidA), m_uuidB(uuidB) {}

bool FType::operator==(const FType &rhs) const {
  return m_uuidA == rhs.m_uuidA && m_uuidB == rhs.m_uuidB;
}

bool FType::operator!=(const FType &rhs) const { return !operator==(rhs); }
}
