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

#include "intermedia/globalConstant.hpp"

#include "compiler/compiler.hpp"

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

  auto ast = m_graph->node("[AST] " + name);
  auto i0  = m_graph->node("[I0] " + name);

  auto compileRule = fpp::rule(m_compiler, &FPCompiler::compileDAssign);

  auto compile = m_graph->edge("compile", compileRule);

  auto astData = ast->data(assign);
  auto i0Data  = i0->data<fie::FIGlobalConstant *>(nullptr);

  astData >> compileRule >> i0Data;

  ast >> compile >> i0;
}

void FPScheduler::schedule(const fps::FInputs *inputs) {
  Walker walker(this);

  walker.walk(inputs);
}
} // namespace pre
