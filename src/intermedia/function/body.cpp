/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (body.cpp)
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

#include "body.hpp"

#include <unordered_set>

#include "evalContext.hpp"
#include "intermedia/context.hpp"

namespace fie {
FIValue *FIFunctionBody::eval(FIContext &ctx) const {
  std::unordered_set<FIBlock *> visited;
  FIBlock *                     next = m_entry;
  FIEvalContext                 state;

  while (true) {
    if (visited.find(next) != visited.end())
      ctx.logger().errorR(m_loc, "loop detected");

    for (auto &ins : next->body()) {
      switch (ins.type()) {
      case FIInstruction::ERR: abort();
      case FIInstruction::Drop: break; // We (probably) don't care about this
      case FIInstruction::Store:
        state.regs[ins.reg()] = ins.value()->eval(ctx, state);
        break;
      }
    }

    state.prev = next;

    switch (next->cont()) {
    case FIBlock::ERR: abort();
    case FIBlock::Static: next = next->next(); break;
    case FIBlock::Branch: {
      auto *val = dynamic_cast<FIBoolConstValue *>(state.regs[next->reg()]);
      if (!val) ctx.logger().errorR(m_loc, "bad branch");
      next = val->value() ? next->next() : next->Else();
      break;
    }
    case FIBlock::Return: return state.regs[next->reg()];
    }
  }
}
} // namespace fie
