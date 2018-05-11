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

// TODO: Remove this?
#include <iostream>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "pipeline/depsGraph.hpp"

#include "constFolder.hpp"
#include "dump.hpp"
#include "llvmCompiler.hpp"

namespace fie {
class FIScheduler {
  struct ScheduleContext {
    struct ConstInfo {
      fpp::FDepsNodeHelper<FIConst *>       node;
      fpp::FDepsNodeHelper<FIFoldedConst *> folded;

      fpp::FDepsEdgeHelper<FIConstFolder, FIFoldedConst *, FIConst *> fold;
    };

    std::unordered_map<FIConst *, ConstInfo> m_consts;
  };

  FIContext *m_ctx;

  fpp::FDepsGraph *         m_graph;
  fun::FPtr<FIDump>         m_dump;
  fun::FPtr<FIConstFolder>  m_constFolder;
  fun::FPtr<FILLVMCompiler> m_llvmCompiler;

  ScheduleContext *m_sctx;

  void scheduleGlobalConst(FIConst *);

  void walkBlock(const fpp::FDepsEdgeOrderHelper &, FIBlock *);

  void walkConst(FIConst *);

  void walkFuncBody(const fpp::FDepsEdgeOrderHelper &, FIFunctionBody &);

  void walkValue(const fpp::FDepsEdgeOrderHelper &, FIValue *);

public:
  constexpr auto &dump() const { return m_dump; }
  constexpr auto &constFolder() const { return m_constFolder; }
  constexpr auto &llvmCompiler() const { return m_llvmCompiler; }

  FIScheduler(fpp::FDepsGraph &  graph,
              FIContext &        ctx,
              const std::string &moduleName) :
      m_ctx(&ctx),
      m_graph(&graph),
      m_dump(fnew<FIDump>(ctx, std::cerr)),
      m_constFolder(fnew<FIConstFolder>(ctx)),
      m_llvmCompiler(fnew<FILLVMCompiler>(ctx, moduleName)) {}

  void schedule();
};
} // namespace fie
