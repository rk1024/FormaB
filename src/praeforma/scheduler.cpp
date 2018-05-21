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

  virtual void visit(const fps::FPDSyntax *syntax) override {
    m_sched->scheduleDSyntax(syntax);
  }

public:
  Walker(FPScheduler *sched) : WalkerBase(sched) {}
};

void FPScheduler::scheduleDSyntax(const fps::FPDSyntax *syntax) {
  m_ctx->logger().infoV("scheduler", syntax->toString());
}

void FPScheduler::scheduleDAssign(const fps::FPDAssign *assign) {
  auto  name = "global '" + assign->name()->value() + "'";
  auto &loc  = assign->loc();

  auto ast = m_graph->node("[AST] " + name, loc, assign);
  auto i0  = m_graph->node<fie::FIConst *>("[I0] " + name, loc, nullptr);

  auto compile = m_graph->edge("compile",
                               m_compiler,
                               &FPCompiler::compileDAssign);

  ast >> compile >> i0;
}

void FPScheduler::schedule(const fps::FInputs *inputs) {
  Walker walker(this);

  walker.walk(inputs);
}
} // namespace pre
