/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (value.cpp)
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

#include "value.hpp"

namespace fie {
FIValue::~FIValue() {}

FIValue::Type FIConstValueBase::type() const { return Const; }

FIValue::Type FIMsgValue::type() const { return Msg; }

FIValue *FIMsgValue::eval(FIContext &ctx, const FIEvalContext &state) const {
  return m_msg->eval(ctx, state, m_params);
}

FIValue::Type FIPhiValue::type() const { return Phi; }

FIValue *FIPhiValue::eval(FIContext &, const FIEvalContext &state) const {
  for (auto &[blk, reg] : m_values) {
    if (blk == state.prev) {
      if (auto it = state.regs.find(reg); it != state.regs.end())
        return it->second;
      return nullptr;
    }
  }

  return nullptr;
}

FIValue::Type FIVarValue::type() const { return Var; }

FIValue *FIVarValue::eval(FIContext &ctx, const FIEvalContext &state) const {
  auto tp = m_scope->type(m_name);

  if (tp == typeid(FIConst *)) {
    auto [result, pConst] = m_scope->find<FIConst *>(m_name);
    assert(result == FIScope::Found);

    auto Const = *pConst;

    if (Const->folded()) return Const->folded()->value()->eval(ctx, state);
  }

  return nullptr;
}
} // namespace fie
