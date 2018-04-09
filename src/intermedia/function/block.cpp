/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (block.cpp)
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

#include "block.hpp"

namespace fie {
void FIBlock::contStatic(const fun::FPtr<FIBlock> &cont) {
  m_cont  = Static;
  m_contA = fun::weak(cont);
  m_contB = nullptr;
  m_ret   = FIRegId(-1);
}

void FIBlock::contBranch(const FIRegId &           cond,
                         bool                      invert,
                         const fun::FPtr<FIBlock> &a,
                         const fun::FPtr<FIBlock> &b) {
  m_cont  = Branch;
  m_contA = fun::weak(invert ? b : a);
  m_contB = fun::weak(invert ? a : b);
  m_ret   = cond;
}

void FIBlock::contRet(const FIRegId &ret) {
  m_cont  = Return;
  m_contA = nullptr;
  m_contB = nullptr;
  m_ret   = ret;
}

std::vector<FIRegId> FIBlock::deps() const {
  switch (m_cont) {
  case Branch:
  case Return: return {m_ret};
  default: return {};
  }
}
} // namespace fie
