/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (typeSolver.cpp)
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

#include "typeSolver.hpp"

#include <unordered_set>

#include "pipeline/miniDepsGraph.hpp"

#include "ti.hpp"

namespace fie {
void FITypeSolver::typeFunc(fun::cons_cell<FIFunctionAtom> args) {
  auto &func = m_inputs->assem()->funcs().value(args.get<0>());
  auto  body = func->body();

  struct Job {
    enum { Block, Value } type;
    union {
      fun::FPtr<FIBlock> block;
      FIRegId            reg;
    };

    Job(const decltype(block) &_block) : type(Block), block(_block) {}
    Job(const FIRegId &_reg) : type(Value), reg(_reg) {}

    Job(const Job &other) : type(other.type) {
      switch (type) {
      case Block: new (&block) fun::FPtr<FIBlock>(other.block); break;
      case Value: new (&reg) FIRegId(other.reg); break;
      }
    }

    ~Job() {
      switch (type) {
      case Block: block.~FPtr(); break;
      case Value: break;
      }
    }
  };

  using Graph = fps::FMiniDepsGraph<Job>;

  auto graph = fnew<Graph>();

  std::unordered_map<FIRegId, fun::FPtr<const FIValue>> values;
  std::unordered_map<FIRegId, fun::FPtr<Graph::Node>>   nodes;
  std::unordered_map<fun::FPtr<FIBlock>,
                     fun::FPtr<Graph::Node>,
                     std::hash<fun::FPtr<FIBlock>>,
                     fun::are_same<FIBlock>>
      blkNodes;

  for (auto it = body.blocks.rbegin(); it != body.blocks.rend(); ++it) {
    auto &block = *it;

    blkNodes.emplace(block, graph->node("block " + block->name(), block));

    for (auto it2 = block->body().rbegin(); it2 != block->body().rend();
         ++it2) {
      auto &ins = *it2;

      nodes.emplace(ins.reg(),
                    graph->node(std::to_string(ins.id()) + ":" +
                                    ins.value()->to_string(),
                                ins.reg()));
    }
  }

  for (auto &block : body.blocks) {
    auto blkNode = blkNodes.at(block);

    auto blkDeps = block->deps();

    for (auto &dep : blkDeps) graph->connect(nodes.at(dep), blkNode);

    for (auto &ins : block->body()) {
      values.emplace(ins.reg(), ins.value());

      auto node = nodes.at(ins.reg());

      auto deps = ins.value()->deps();

      for (auto &dep : deps) graph->connect(nodes.at(dep), node);
    }
  }

  TI        t(TIContext(m_inputs, fun::wrap(this), values));
  w::Scheme retType;

  for (auto var : body.vars) {
    t.context.env.emplace(var.second, w::Scheme({}, t.makeVar()));
  }

  // for (auto &pair : func->args()) {
  //   // std::cerr << "ARG v" << pair.first << " :: " << pair.second->name()
  //   //           << std::endl;

  //   if (pair.second == builtins::FIErrorT)
  //     t.context.env[pair.first] = w::generalize(t.context.env, t.makeVar());
  //   else {
  //     t.context.env[pair.first] = w::generalize(
  //         t.context.env, fnew<WTypeStruct>(pair.second, WTypeStruct::Params()));
  //   }
  // }

  auto runJob = [&](const Job &job) {
    switch (job.type) {
    case Job::Value: {
      auto [sb, tp] = t.context.values->at(job.reg)->ti(t);
      t.context.putType(job.reg, tp);
      t.context.subst = w::composeSubst(sb, t.context.subst);
      break;
    }
    case Job::Block: {
      switch (job.block->cont()) {
      case FIBlock::BranchFT:
      case FIBlock::BranchTF: {
        auto tp         = t.context.getType(job.block->ret());
        auto sb         = tp->mgu(fnew<WTypeStruct>(builtins::FIBool,
                                            WTypeStruct::Params()),
                          t);
        t.context.subst = w::composeSubst(sb, t.context.subst);
        break;
      }
      case FIBlock::Return: {
        auto tp = t.context.getType(job.block->ret());

        if (retType.type().nil()) {
          w::TIPos _(t,
                     "\e[1mreturn type\e[0m " + tp->to_string() + " (from '" +
                         t.context.values->at(job.block->ret())->to_string() +
                         "')");

          retType = w::generalize(t.context.env, tp);
        }
        else {
          w::TIPos _(t, "\e[1munify return\e[0m");

          auto sb = tp->mgu(t.instantiate(retType), t);

          retType = w::generalize(t.context.env, w::sub(sb, tp));
        }
        break;
      }
      default: break;
      }
    }
    }
  };

  // graph->dot(std::cout);

  graph->run(runJob);

  // // Now re-unify all the ldvars with their respective variables
  // for (auto &block : body.blocks) {
  //   for (auto &ins : block->body()) {
  //     if (t.context.types.find(ins.value().get()) == t.context.types.end()) {
  //       std::cerr << "WARNING: Missing type info for register " << ins.id()
  //                 << " - '" << ins.value()->to_string() << "'" << std::endl;
  //     }

  //     switch (ins.value()->opcode()) {
  //     case FIOpcode::Ldvar: {
  //       auto     val = ins.value().get();
  //       w::TIPos _(t, "\e[1mreunify\e[0m " + val->to_string());

  //       auto  var   = ins.value().as<const FILdvarValue>()->var();
  //       auto &ldTp  = t.context.types.at(val);
  //       auto  varTp = t.instantiate(t.context.env.at(var));
  //       auto  su    = varTp->mgu(ldTp, t);

  //       t.context.env[var] = w::generalize(t.context.env, w::sub(su, varTp));

  //       t.context.types[val] = w::sub(su, ldTp);

  //       break;
  //     }
  //     default: break;
  //     }
  //   }
  // }

  WTypeStruct::Params typeParams{t.instantiate(retType)};

  for (auto &pair : func->args()) {
    typeParams.push_back(t.instantiate(t.context.getEnv(pair.first)));
  }

  m_funcs[func] = w::generalize(
      t.context.env,
      fnew<WTypeStruct>(fnew<FIStruct>("fun", func->args().size() + 1),
                        typeParams));

  for (auto &pair : values) {
    std::cerr << "\e[1mRESULT:\e[0m " << pair.second->to_string()
              << " :: " << t.context.getType(pair.first)->to_string()
              << std::endl;
  }

  for (auto &pair : t.context.env) {
    std::cerr << "\e[1mENV:\e[0m v" << pair.first << " ("
              << func->body().vars.value(pair.first).name()
              << ") :: " << t.context.getEnv(pair.first).to_string()
              << std::endl;
  }

  std::cerr << "\e[1mRETURN:\e[0m " << retType.to_string() << std::endl;

  std::cerr << "\e[1mFUNCTION TYPE:\e[0m " << m_funcs[func].to_string()
            << std::endl;
}
} // namespace fie
