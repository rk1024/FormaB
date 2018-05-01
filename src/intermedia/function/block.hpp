/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (block.hpp)
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

#include "instruction.hpp"

namespace fie {
class FIBlock {
public:
  enum Continue {
    ERR,
    Static,
    Branch,
    Return,
    // TODO: Maybe add indirect jump or jump tables?
  };

private:
  std::string                m_name;
  std::vector<FIInstruction> m_body;
  Continue                   m_cont;
  FIBlock *                  m_next, *m_else;
  FIRegId                    m_reg;

public:
  constexpr auto &name() const { return m_name; }
  constexpr auto &body() { return m_body; }
  constexpr auto &body() const { return m_body; }
  constexpr auto &cont() { return m_cont; }
  constexpr auto &cont() const { return m_cont; }
  constexpr auto &next() { return m_next; }
  constexpr auto &next() const { return m_next; }
  constexpr auto &Else() { return m_else; }
  constexpr auto &Else() const { return m_else; }
  constexpr auto &reg() { return m_reg; }
  constexpr auto &reg() const { return m_reg; }

  FIBlock(const std::string &name) : m_name(name), m_cont(ERR) {}
};
} // namespace fie
