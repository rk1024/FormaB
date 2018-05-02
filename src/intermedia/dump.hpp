/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (dump.hpp)
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

#include <ostream>

#include "util/object/object.hpp"

#include "context.hpp"

namespace fie {
class FIDump : public fun::FObject {
  FIContext *   m_ctx;
  std::ostream *m_os;

  void writeBlock(const FIBlock *) const;

  void writeConst(const FIConst *) const;

  void writeFoldedConst(const FIFoldedConst *) const;

  void writeFuncBody(const FIFunctionBody *) const;

  void writeReg(const FIRegId &) const;

  void writeValue(const FIValue *) const;

public:
  constexpr auto &os() const { return *m_os; }

  FIDump(FIContext &ctx, std::ostream &os) : m_ctx(&ctx), m_os(&os) {}

  void dumpConst(FIConst *);

  void dumpFoldedConst(FIFoldedConst *);
};
} // namespace fie
