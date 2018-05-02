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

#include "ast/walker.hpp"

#include "intermedia/const.hpp"

namespace pre {
class FPScheduler::WalkerBase : public fps::FWalker {
protected:
  FPScheduler *m_sched;

public:
  WalkerBase(FPScheduler *sched) : m_sched(sched) {}
};

class FPScheduler::Walker : public WalkerBase {
protected:
  virtual void visit(const fps::FPDAssign *assign) override {
    m_sched->scheduleDAssign(assign);
  }

public:
  Walker(FPScheduler *sched) : WalkerBase(sched) {}
};

void FPScheduler::scheduleDAssign(const fps::FPDAssign *assign) {
  auto name = "global '" + assign->name()->value() + "'";

  auto ast    = m_graph->node("[AST] " + name, assign);
  auto i0     = m_graph->node<fie::FIConst *>("[I0] " + name, nullptr);
  auto folded = m_graph->node<fie::FIFoldedConst *>("[Folded] " + name,
                                                    nullptr);

  auto compile = m_graph->edge("compile",
                               m_compiler,
                               &FPCompiler::compileDAssign);
  auto fold    = m_graph->edge("fold",
                            m_constFolder,
                            &fie::FIConstFolder::foldConstant);

  ast >> compile >> i0 >> fold >> folded;

#if !defined(NDEBUG)
  auto printed = m_graph->node<void>("[Printed] " + name);

  auto dump       = m_graph->edge("dump", m_dump, &fie::FIDump::dumpConst);
  auto dumpFolded = m_graph->edge("dumpFolded",
                                  m_dump,
                                  &fie::FIDump::dumpFoldedConst);

  i0 >> dump >> printed;
  folded >> dumpFolded >> printed;
#endif
}

void FPScheduler::schedule(const fps::FInputs *inputs) {
  Walker walker(this);

  walker.walk(inputs);
}
} // namespace pre
