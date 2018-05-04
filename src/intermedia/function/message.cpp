/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (message.cpp)
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

#include "message.hpp"

#include "intermedia/context.hpp"

namespace fie {
FIMessageBase::~FIMessageBase() {}

std::string FIAddMessage::name() const { return "o@op:+:"; }

FIValue *FIAddMessage::eval(FIContext &                 ctx,
                            const FIEvalContext &       state,
                            const std::vector<FIRegId> &params) const {
  if (params.size() != 2) return nullptr;

  auto *lhs = dynamic_cast<FIDoubleConstValue *>(state.regs.at(params[0]));
  auto *rhs = dynamic_cast<FIDoubleConstValue *>(state.regs.at(params[1]));

  if (!(lhs && rhs)) return nullptr;

  // TODO: Watch your arithmetic!
  return ctx.val<FIDoubleConstValue>(m_loc, lhs->value() + rhs->value());
}

void FIAddMessage::emit(FIContext &ctx, llvm::IRBuilder<> &) const {
  ctx.logger().fatalR("msg-add", "emit not implemented");
}
} // namespace fie
