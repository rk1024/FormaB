#pragma once

#include <queue>
#include <unordered_map>

#include "util/cons.hpp"

#include "pipeline/depsGraph.hpp"

#include "ast/ast.hpp"

#include "inputs.hpp"

namespace fie {
class FIPraeCompiler;

class FIScheduler : public fun::FObject {
  class WalkerBase;
  class Walker;
  class DepWalker;

  struct Pipeline : fun::FObject {
    fun::FPtr<fps::FDepsGraphNode> input, output;
  };

  struct FuncPipeline : Pipeline {
    fun::FPtr<fps::FDepsGraphEdge> compile;
  };

  struct EntryPointPipeline : FuncPipeline {};

  fun::FPtr<fps::FDepsGraph> m_graph;
  fun::FPtr<FIInputs>        m_inputs;
  fun::FPtr<FIPraeCompiler>  m_compiler;

  fun::FAtomStore<const frma::FPXFunc *> m_funcIds;
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

public:
  FIScheduler(fun::FPtr<fps::FDepsGraph>,
              fun::FPtr<FIInputs>,
              fun::FPtr<FIPraeCompiler>);

  void schedule(const frma::FPrims *);
};
}