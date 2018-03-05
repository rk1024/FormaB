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

#include <cassert>

#include "ast/walker.hpp"

#include "debug/dumpFunction.hpp"
#include "llvmEmitter/llvmEmitter.hpp"
#include "optimizer/optimizer.hpp"
#include "praeCompiler/compiler.hpp"
#include "verifier/verifier.hpp"

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
  virtual void visit(const FPBlock *blk) override {
    assert(blk->alt() != FPBlock::Error);

    m_sched->scheduleEntryPoint(blk->stmts());
  }

  virtual void visit(const FPXFunc *func) override {
    m_sched->scheduleFunc(func);
  }

public:
  Walker(fun::FPtr<FIScheduler> sched) : WalkerBase(sched) {}
};

class FIScheduler::DepWalker : public WalkerBase, public fun::FObject {
  fun::FPtr<fps::FDepsGraphEdge> m_edge;

protected:
  virtual void visit(const FPXFunc *func) override {
    m_sched->m_funcs.at(func)->output >> m_edge;
  }

public:
  DepWalker(fun::FPtr<FIScheduler> sched, fun::FPtr<fps::FDepsGraphEdge> edge) :
      WalkerBase(sched),
      m_edge(edge) {}
};

void FIScheduler::scheduleEntryPoint(const FPStmts *stmts) {
  auto pipe = fnew<EntryPointPipeline>();

  auto name = "Entry point " +
              std::to_string(m_entryPointIds.intern(stmts).value() + 1);

  pipe->input    = m_graph->node("[AST] " + name);
  pipe->compiled = m_graph->node("[Compiled] " + name);
  pipe->verified = m_graph->node("[Verified] " + name);
  pipe->output   = m_graph->node("[Done] " + name);

  auto compiledData = pipe->compiled->data<FIFunctionAtom>(FIFunctionAtom(-1));

  auto compileRule = fps::rule(m_compiler, &FIPraeCompiler::compileEntryPoint);
  auto verifyRule  = fps::rule(m_verifier, &FIVerifier::verifyFunc);
  auto dumpRule    = fps::rule(m_dumpFunc, &FIDumpFunction::dumpFunc);
  auto emitRule    = fps::rule(m_emitter, &FILLVMEmitter::emitEntryPoint);

  pipe->input->data(stmts) >> compileRule >> compiledData;
  compiledData >> verifyRule;
  compiledData >> dumpRule;
  compiledData >> emitRule;

  pipe->compile = m_graph->edge("compile", compileRule);
  pipe->verify  = m_graph->edge("verify", verifyRule);
  pipe->dump    = m_graph->edge("dump", dumpRule);
  pipe->emit    = m_graph->edge("emit", emitRule);

  pipe->input >> pipe->compile >> pipe->compiled >> pipe->verify >>
      pipe->verified >> pipe->dump >> pipe->output;

  pipe->verified >> pipe->emit >> pipe->output;

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

  auto name = "Function " + std::to_string(m_funcIds.intern(func).value() + 1);

  pipe->input    = m_graph->node("[AST] " + name);
  pipe->compiled = m_graph->node("[Compiled] " + name);
  pipe->verified = m_graph->node("[Verified] " + name);
  pipe->output   = m_graph->node("[Func] " + name);

  auto compiledData = pipe->compiled->data<FIFunctionAtom>(FIFunctionAtom(-1));

  auto compileRule = fps::rule(m_compiler, &FIPraeCompiler::compileFunc);
  auto verifyRule  = fps::rule(m_verifier, &FIVerifier::verifyFunc);
  auto dumpRule    = fps::rule(m_dumpFunc, &FIDumpFunction::dumpFunc);
  auto emitRule    = fps::rule(m_emitter, &FILLVMEmitter::emitFunc);

  pipe->input->data(func) >> compileRule >> compiledData;
  compiledData >> verifyRule;
  compiledData >> dumpRule;
  compiledData >> emitRule;

  pipe->compile = m_graph->edge("compile", compileRule);
  pipe->verify  = m_graph->edge("verify", verifyRule);
  pipe->dump    = m_graph->edge("dump", dumpRule);
  pipe->emit    = m_graph->edge("emit", emitRule);

  pipe->input >> pipe->compile >> pipe->compiled >> pipe->verify >>
      pipe->verified >> pipe->dump >> pipe->output;

  pipe->verified >> pipe->emit >> pipe->output;

  m_funcs[func] = pipe;

  m_depQ.push(
      fun::cons(fnew<DepWalker>(fun::wrap(this), pipe->compile),
                reinterpret_cast<void (DepWalker::*)(const void *, bool)>(
                    static_cast<void (DepWalker::*)(const FPXFunc *, bool)>(
                        &DepWalker::walk)),
                reinterpret_cast<const void *>(func)));
}

void FIScheduler::scheduleType(const FPDType *type) {
  auto pipe = fnew<TypePipeline>();

  auto name = "Type " + std::to_string(m_typeIds.intern(type).value() + 1);

  pipe->input  = m_graph->node("[AST] " + name);
  pipe->output = m_graph->node("[Type] " + name);

  auto compileRule = fps::rule(m_compiler, &FIPraeCompiler::compileType);

  pipe->compile = m_graph->edge("compile", compileRule);
}

FIScheduler::FIScheduler(fun::FPtr<fps::FDepsGraph> graph,
                         fun::FPtr<FIInputs>        inputs) :
    m_graph(graph),
    m_inputs(inputs),
    m_compiler(fnew<FIPraeCompiler>(inputs)),
    m_verifier(fnew<FIVerifier>(inputs)),
    m_dumpFunc(fnew<FIDumpFunction>(inputs, std::cerr)),
    m_emitter(fnew<FILLVMEmitter>(inputs)) {}

void FIScheduler::schedule(const frma::FPrims *prims) {
  Walker walker(fun::wrap(this));

  walker.walk(prims);

  while (m_depQ.size()) {
    auto cell = m_depQ.front();
    (cell.get<0>()->*cell.get<1>())(cell.get<2>(), true);
    m_depQ.pop();
  }
}
} // namespace fie
