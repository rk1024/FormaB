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

#include "compiler.hpp"

namespace pre {
class FPScheduler {
  class WalkerBase;
  class Walker;

  FPContext *m_ctx;

  fpp::FDepsGraph *     m_graph;
  fun::FPtr<FPCompiler> m_compiler;

  void scheduleDSyntax(const fps::FPDSyntax *);

  void scheduleDAssign(const fps::FPDAssign *);

public:
  constexpr auto &compiler() const { return m_compiler; }

  FPScheduler(fpp::FDepsGraph &graph, FPContext &ctx) :
      m_ctx(&ctx),
      m_graph(&graph),
      m_compiler(fnew<FPCompiler>(ctx)) {}

  void schedule(const fps::FInputs *);
};
} // namespace pre
