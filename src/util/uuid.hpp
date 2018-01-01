/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (uuid.hpp)
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
} // namespace fun

namespace std {
template <>
struct hash<fun::UUID> {
  inline std::size_t operator()(const fun::UUID &uuid) const {
    return hash<decltype(uuid.m_uuid)>{}(uuid.m_uuid);
  }
};
} // namespace std
