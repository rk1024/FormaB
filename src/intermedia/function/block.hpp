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

#include <vector>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "instruction.hpp"

namespace fie {
class FIBlock : public fun::FObject {
public:
  // NOTE: BranchTF: contA is true, contB is false
  //       BranchFT: contB is true, contA is false
  enum Continue { Static, BranchTF, BranchFT, Return, ERR = -1 };

private:
  std::string                m_name;
  Continue                   m_cont;
  fun::FWeakPtr<FIBlock>     m_contA, m_contB;
  FIRegId                    m_ret;
  std::vector<FIInstruction> m_body;

public:
  constexpr auto &name() const { return m_name; }
  constexpr auto &cont() const { return m_cont; }
  constexpr auto &contA() const { return m_contA; }
  constexpr auto &contB() const { return m_contB; }
  constexpr auto &ret() const { return m_ret; }
  constexpr auto &body() { return m_body; }

  FIBlock(const std::string &name) : m_name(name), m_cont(ERR), m_ret(-1) {}

  void contStatic(const fun::FPtr<FIBlock> &);

  void contBranch(const FIRegId &,
                  bool,
                  const fun::FPtr<FIBlock> &,
                  const fun::FPtr<FIBlock> &);

  void contRet(const FIRegId &);

  std::vector<FIRegId> deps() const;
};
} // namespace fie
