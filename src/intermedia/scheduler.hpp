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

#include <queue>
#include <unordered_map>

#include "util/cons.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "pipeline/depsGraph.hpp"

#include "ast/ast.hpp"

#include "inputs.hpp"

namespace fie {
class FIPraeCompiler;
class FIVerifier;
class FIDumpFunction;

class FIScheduler : public fun::FObject {
  class WalkerBase;
  class Walker;
  class DepWalker;

  struct Pipeline : fun::FObject {
    fun::FPtr<fps::FDepsGraphNode> input, output;
  };

  struct CompilePipeline : fun::FObject {
    fun::FPtr<fps::FDepsGraphNode> input, output;
    fun::FPtr<fps::FDepsGraphEdge> compile;
  };

  struct FuncPipeline : CompilePipeline {
    fun::FPtr<fps::FDepsGraphNode> compiled, verified;
    fun::FPtr<fps::FDepsGraphEdge> verify, dump;
  };

  struct EntryPointPipeline : FuncPipeline {};

  struct TypePipeline : CompilePipeline {};

  fun::FPtr<fps::FDepsGraph> m_graph;
  fun::FPtr<FIInputs>        m_inputs;
  fun::FPtr<FIPraeCompiler>  m_compiler;
  fun::FPtr<FIVerifier>      m_verifier;
  fun::FPtr<FIDumpFunction>  m_dumpFunc;

  fun::FAtomStore<const frma::FPXFunc *> m_funcIds;
  fun::FAtomStore<const frma::FPDType *> m_typeIds;
  fun::FAtomStore<const frma::FPStmts *> m_entryPointIds;

  std::unordered_map<const frma::FPXFunc *, fun::FPtr<FuncPipeline>> m_funcs;
  std::unordered_map<const frma::FPStmts *, fun::FPtr<EntryPointPipeline>>
      m_entryPoints;

  std::queue<fun::cons_cell<fun::FPtr<DepWalker>,
                            void (DepWalker::*)(const void *, bool),
                            const void *>>
      m_depQ;

  void scheduleEntryPoint(const frma::FPStmts *);

  void scheduleFunc(const frma::FPXFunc *);

  void scheduleType(const frma::FPDType *);

public:
  FIScheduler(fun::FPtr<fps::FDepsGraph>, fun::FPtr<FIInputs>);

  void schedule(const frma::FPrims *);
};
} // namespace fie
