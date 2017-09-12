#include "scheduler.hpp"

#include <cassert>

#include "pipeline/transfer.hpp"

#include "ast/walker.hpp"

#include "praeCompiler/compiler.hpp"

using namespace frma;

namespace fie {
class FIScheduler::WalkerBase : public FWalker {
protected:
  fun::FPtr<FIScheduler> m_sched;

public:
  WalkerBase(fun::FPtr<FIScheduler> sched) : m_sched(sched) {}
};

class FIScheduler::Walker : public WalkerBase {
protected:
  virtual void visit(const FPBlock *blk) {
    assert(blk->alt() != FPBlock::Error);

    m_sched->scheduleEntryPoint(blk->stmts());
  }

  virtual void visit(const FPXFunc *func) { m_sched->scheduleFunc(func); }

public:
  Walker(fun::FPtr<FIScheduler> sched) : WalkerBase(sched) {}
};

class FIScheduler::DepWalker : public WalkerBase, public fun::FObject {
  fun::FPtr<fps::FDepsGraphEdge> m_edge;

protected:
  virtual void visit(const FPXFunc *func) {
    m_sched->m_funcs.at(func)->output >> m_edge;
  }

public:
  DepWalker(fun::FPtr<FIScheduler> sched, fun::FPtr<fps::FDepsGraphEdge> edge)
      : WalkerBase(sched), m_edge(edge) {}
};

void FIScheduler::scheduleEntryPoint(const FPStmts *stmts) {
  auto pipe = fnew<EntryPointPipeline>();

  auto id = std::to_string(m_entryPointIds.intern(stmts) + 1);

  pipe->input  = m_graph->node("[AST] Entry point " + id);
  pipe->output = m_graph->node("[Func] Entry point " + id);

  pipe->compile = m_graph->edge("compile", fps::transfer(stmts, m_compiler));

  pipe->input >> pipe->compile >> pipe->output;

  m_entryPoints[stmts] = pipe;

  m_depQ.push(
      fun::cons(fnew<DepWalker>(fun::wrap(this), pipe->compile),
                reinterpret_cast<void (DepWalker::*)(const void *, bool)>(
                    static_cast<void (DepWalker::*)(const FPStmts *, bool)>(
                        &DepWalker::walk)),
                reinterpret_cast<const void *>(stmts)));
}

void FIScheduler::scheduleFunc(const FPXFunc *func) {
  auto pipe = fnew<FuncPipeline>();

  auto name = "Function " + std::to_string(m_funcIds.intern(func) + 1);

  pipe->input  = m_graph->node("[AST] " + name);
  pipe->output = m_graph->node("[Func] " + name);

  pipe->compile = m_graph->edge("compile", fps::transfer(func, m_compiler));

  pipe->input >> pipe->compile >> pipe->output;

  m_funcs[func] = pipe;

  m_depQ.push(
      fun::cons(fnew<DepWalker>(fun::wrap(this), pipe->compile),
                reinterpret_cast<void (DepWalker::*)(const void *, bool)>(
                    static_cast<void (DepWalker::*)(const FPXFunc *, bool)>(
                        &DepWalker::walk)),
                reinterpret_cast<const void *>(func)));
}

FIScheduler::FIScheduler(fun::FPtr<fps::FDepsGraph> graph,
                         fun::FPtr<FIInputs>        inputs,
                         fun::FPtr<FIPraeCompiler>  compiler)
    : m_graph(graph), m_inputs(inputs), m_compiler(compiler) {}

void FIScheduler::schedule(const frma::FPrims *prims) {
  Walker walker(fun::wrap(this));

  walker.walk(prims);

  while (m_depQ.size()) {
    auto cell = m_depQ.front();
    (cell.get<0>()->*cell.get<1>())(cell.get<2>(), true);
    m_depQ.pop();
  }
}
}
