/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scheduler.cpp)
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

#include "scheduler.hpp"

namespace fie {
void FIScheduler::scheduleGlobalConst(FIConst *Const) {
  auto  name = "global '" + Const->name() + "'";
  auto &loc  = Const->body().loc();

  auto i0     = m_graph->node<FIConst *>("[I0] " + name, loc, Const);
  auto folded = m_graph->node<FIFoldedConst *>("[Folded] " + name,
                                               loc,
                                               nullptr);

  auto fold = m_graph->edge("fold",
                            m_constFolder,
                            &FIConstFolder::foldConstant);

  i0 >> fold >> folded;

  m_sctx->m_consts.emplace(Const,
                           ScheduleContext::ConstInfo{.node   = i0,
                                                      .folded = folded,
                                                      .fold   = fold});

#if !defined(NDEBUG)
  auto printed = m_graph->node<void>("[Printed] " + name, loc);

  auto dump       = m_graph->edge("dump", m_dump, &FIDump::dumpConst);
  auto dumpFolded = m_graph->edge("dumpFolded",
                                  m_dump,
                                  &fie::FIDump::dumpFoldedConst);

  i0 >> dump >> printed;
  folded >> dumpFolded >> printed;
#endif
}

void FIScheduler::walkBlock(const fpp::FDepsEdgeOrderHelper &order,
                            FIBlock *                        block) {
  for (auto &ins : block->body()) walkValue(order, ins.value());
}

void FIScheduler::walkConst(FIConst *Const) {
  walkFuncBody(m_sctx->m_consts.at(Const).fold.order(), Const->body());
}

void FIScheduler::walkFuncBody(const fpp::FDepsEdgeOrderHelper &order,
                               FIFunctionBody &                 body) {
  for (auto &block : body.blocks()) walkBlock(order, block);
}

void FIScheduler::walkValue(const fpp::FDepsEdgeOrderHelper &order,
                            FIValue *                        value) {
  switch (value->type()) {
  case FIValue::Var: {
    auto *var = dynamic_cast<FIVarValue *>(value);

    if (auto [result, Const] = var->scope()->find<FIConst *>(var->name());
        result == FIScope::Found) {
      m_sctx->m_consts.at(*Const).folded.order() >> order;
    }

    break;
  }
  default: break;
  }
}

void FIScheduler::schedule() {
  m_sctx = new ScheduleContext{};

  for (auto &[name, Const] :
       m_ctx->module().globals()->container<FIConst *>()->map())
    scheduleGlobalConst(Const);

  for (auto &[name, Const] :
       m_ctx->module().globals()->container<FIConst *>()->map())
    walkConst(Const);

  delete m_sctx;
}
} // namespace fie
