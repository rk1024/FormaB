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

  auto ast    = m_graph->node("[AST] " + name);
  auto i0     = m_graph->node("[I0] " + name);
  auto folded = m_graph->node("[Folded] " + name);
  // #if !defined(NDEBUG)
  //   auto dumped = m_graph->node("[Dumped] " + name);
  // #endif

  auto compileRule = fpp::rule(m_compiler, &FPCompiler::compileDAssign);
  auto foldRule = fpp::rule(m_constFolder, &fie::FIConstFolder::foldConstant);
  // #if !defined(NDEBUG)
  //   auto dumpRule = fpp::rule(m_dump, &fie::FIDump::dumpConst);
  // #endif

  auto compile = m_graph->edge("compile", compileRule);
  auto fold    = m_graph->edge("fold", foldRule);
  // #if !defined(NDEBUG)
  //   auto dump = m_graph->edge("dump", dumpRule);
  // #endif

  auto astData    = ast->data(assign);
  auto i0Data     = i0->data<fie::FIConst *>(nullptr);
  auto foldedData = folded->data<fie::FIFoldedConst *>(nullptr);

  astData >> compileRule >> i0Data >> foldRule >> foldedData;

  ast >> compile >> i0 >> fold >> folded;

  // #if defined(NDEBUG)
  // #else
  //   astData >> compileRule >> i0Data >> dumpRule;

  //   ast >> compile >> i0 >> dump >> dumped;
  // #endif
}

void FPScheduler::schedule(const fps::FInputs *inputs) {
  Walker walker(this);

  walker.walk(inputs);
}
} // namespace pre
