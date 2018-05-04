/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (message.hpp)
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

#include <llvm/IR/IRBuilder.h>

#include "diagnostic/location.hpp"

#include "evalContext.hpp"

namespace fie {
class FIContext;
class FIValue;

class FIMessageBase {
protected:
  fdi::FLocation m_loc;

public:
  constexpr auto &loc() const { return m_loc; }

  FIMessageBase(const fdi::FLocation &loc) : m_loc(loc) {}

  virtual ~FIMessageBase();

  virtual std::string name() const = 0;

  virtual FIValue *eval(FIContext &,
                        const FIEvalContext &,
                        const std::vector<FIRegId> &) const = 0;

  virtual void emit(FIContext &, llvm::IRBuilder<> &) const = 0;
};

class FIAddMessage : public FIMessageBase {
public:
  FIAddMessage(const fdi::FLocation &loc) : FIMessageBase(loc) {}

  virtual std::string name() const override;

  virtual FIValue *eval(FIContext &,
                        const FIEvalContext &,
                        const std::vector<FIRegId> &) const override;

  virtual void emit(FIContext &, llvm::IRBuilder<> &) const override;
};
} // namespace fie
