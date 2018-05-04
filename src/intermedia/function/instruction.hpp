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

#include "regId.hpp"

namespace fie {
class FIValue;

class FIInstruction {
public:
  enum Type {
    ERR,
    Drop,
    Store,
  };

private:
  Type m_type;

  union {
    struct {
      FIValue *value;
    } m_drop;

    struct {
      FIRegId  reg;
      FIValue *value;
    } m_store;
  };

public:
  constexpr auto &type() const { return m_type; }

  constexpr auto &value() const {
    switch (m_type) {
    case ERR: abort();
    case Drop: return m_drop.value;
    case Store: return m_store.value;
    }
  }

  constexpr auto &reg() const {
    switch (m_type) {
    case ERR:
    case Drop: abort();
    case Store: return m_store.reg;
    }
  }

  FIInstruction() : m_type(ERR) {}

  FIInstruction(FIValue *value) : m_type(Drop), m_drop{value} {}

  FIInstruction(FIRegId reg, FIValue *value) :
      m_type(Store),
      m_store{reg, value} {}

  FIInstruction(const FIInstruction &rhs) : m_type(rhs.m_type) {
    switch (m_type) {
    case ERR: break;
    case Drop: new (&m_drop) decltype(m_drop)(rhs.m_drop); break;
    case Store: new (&m_store) decltype(m_store)(rhs.m_store); break;
    }
  }

  FIInstruction(FIInstruction &&rhs) : m_type(rhs.m_type) {
    switch (m_type) {
    case ERR: break;
    case Drop: new (&m_drop) decltype(m_drop)(std::move(rhs.m_drop)); break;
    case Store: new (&m_store) decltype(m_store)(std::move(rhs.m_store)); break;
    }

    rhs.m_type = ERR;
  }

  ~FIInstruction() {}
};
} // namespace fie
