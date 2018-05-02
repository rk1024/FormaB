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

#include <iostream>
#include <queue>

#include "pipeline/depsGraph.hpp"

#include "parser/ast.hpp"

#include "intermedia/constFolder.hpp"
#include "intermedia/dump.hpp"
#include "intermedia/llvmCompiler.hpp"

#include "compiler.hpp"

namespace pre {
class FPScheduler : public fun::FObject {
  class WalkerBase;
  class Walker;

  fun::FPtr<fpp::FDepsGraph> m_graph;
  fun::FPtr<FPCompiler>      m_compiler;
  fun::FPtr<fie::FIDump>     m_dump;
  // TODO: Maybe make the Intermedia pipeline separate?
  fun::FPtr<fie::FIConstFolder>  m_constFolder;
  fun::FPtr<fie::FILLVMCompiler> m_llvmCompiler;

  void scheduleDAssign(const fps::FPDAssign *);

public:
  constexpr auto &compiler() const { return m_compiler; }
  constexpr auto &dump() const { return m_dump; }
  constexpr auto &constFolder() const { return m_constFolder; }
  constexpr auto &llvmCompiler() const { return m_llvmCompiler; }

  FPScheduler(const fun::FPtr<fpp::FDepsGraph> &graph,
              FPContext &                       ctx,
              const std::string &               moduleName) :
      m_graph(graph),
      m_compiler(fnew<FPCompiler>(ctx)),
      m_dump(fnew<fie::FIDump>(ctx.fiCtx(), std::cerr)),
      m_constFolder(fnew<fie::FIConstFolder>(ctx.fiCtx())),
      m_llvmCompiler(fnew<fie::FILLVMCompiler>(ctx.fiCtx(), moduleName)) {}

  void schedule(const fps::FInputs *);
};
} // namespace pre
