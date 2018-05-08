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
void FIScheduler::scheduleGlobalConst(
    const std::string &name, fpp::FDepsNodeHelper<fie::FIConst *> &Const) {
  auto folded = m_graph->node<FIFoldedConst *>("[Folded] " + name, nullptr);

  auto fold = m_graph->edge("fold",
                            m_constFolder,
                            &FIConstFolder::foldConstant);

  Const >> fold >> folded;

  Const.order() >> m_sctx->unresolvedScope;
  m_sctx->resolvedScope.order() >> fold;

#if !defined(NDEBUG)
  auto printed = m_graph->node<void>("[Printed] " + name);

  auto dump       = m_graph->edge("dump", m_dump, &FIDump::dumpConst);
  auto dumpFolded = m_graph->edge("dumpFolded",
                                  m_dump,
                                  &fie::FIDump::dumpFoldedConst);

  Const >> dump >> printed.order();
  folded >> dumpFolded >> printed.order();
#endif
}

void FIScheduler::schedule() {
  m_sctx = new ScheduleContext{
      .unresolvedScope = m_graph->node<void>("(scope unresolved)"),
      .resolvedScope   = m_graph->node<void>("(scope resolved)"),
  };

  // auto resolveScope = m_graph->edge("resolveScope",
  //                                   fun::wrap(this),
  //                                   &FIScheduler::resolveScope);

  // m_sctx->unresolvedScope.order() >> resolveScope >> m_sctx->resolvedScope;

  delete m_sctx;
}
} // namespace fie
