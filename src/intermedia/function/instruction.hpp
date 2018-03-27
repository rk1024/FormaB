/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (instruction.hpp)
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

#include "util/ptr.hpp"

#include "regId.hpp"
#include "values.hpp"

namespace fie {
class FIInstruction {
  std::string              m_name;
  std::uint32_t            m_id;
  fun::FPtr<const FIValue> m_value;

public:
  constexpr auto &name() const { return m_name; }
  constexpr auto &id() const { return m_id; }
  constexpr auto &value() const { return m_value; }

  FIRegId reg() const { return FIRegId(m_id); }

  FIInstruction(const std::string &             name,
                std::uint32_t                   id,
                const fun::FPtr<const FIValue> &value) :
      m_name(name),
      m_id(id),
      m_value(value) {}
};
} // namespace fie
