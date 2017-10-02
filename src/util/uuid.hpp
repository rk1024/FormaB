#pragma once

#include <cstdint>
#include <utility>

#include "cons.hpp"

namespace fun {
class UUID {
  const cons_cell<std::int64_t, std::int64_t> m_uuid;

public:
  UUID(cons_cell<std::int64_t, std::int64_t> uuid) : m_uuid(uuid) {}

  UUID(std::int64_t a, std::int64_t b) : UUID(cons(a, b)) {}

  bool operator==(const UUID &rhs) { return m_uuid == rhs.m_uuid; }

  bool operator!=(const UUID &rhs) { return m_uuid != rhs.m_uuid; }

  friend struct std::hash<fun::UUID>;
};
}

namespace std {
template <>
struct hash<fun::UUID> {
  inline std::size_t operator()(const fun::UUID &uuid) const {
    return hash<decltype(uuid.m_uuid)>{}(uuid.m_uuid);
  }
};
}
