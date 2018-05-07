/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scheduler.hpp)
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

#include "pipeline/depsGraph.hpp"

#include "parser/ast.hpp"

#include "intermedia/scheduler.hpp"

#include "compiler.hpp"

namespace pre {
class FPScheduler : public fun::FObject {
  class WalkerBase;
  class Walker;

  FPContext *m_ctx;

  fun::FPtr<fpp::FDepsGraph>  m_graph;
  fun::FPtr<FPCompiler>       m_compiler;
  fun::FPtr<fie::FIScheduler> m_fiScheduler;

  void scheduleDSyntax(const fps::FPDSyntax *);

  void scheduleDAssign(const fps::FPDAssign *);

public:
  constexpr auto &compiler() const { return m_compiler; }
  constexpr auto &fiScheduler() const { return m_fiScheduler; }

  FPScheduler(const fun::FPtr<fpp::FDepsGraph> &graph,
              FPContext &                       ctx,
              const std::string &               moduleName) :
      m_ctx(&ctx),
      m_graph(graph),
      m_compiler(fnew<FPCompiler>(ctx)),
      m_fiScheduler(fnew<fie::FIScheduler>(graph, ctx.fiCtx(), moduleName)) {}
  void schedule(const fps::FInputs *);
};
} // namespace pre
