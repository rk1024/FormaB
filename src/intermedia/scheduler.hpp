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
class FIScheduler : public fun::FObject {
  struct ScheduleContext {
    fpp::FDepsNodeHelper<void> resolveScope;
  };

  FIContext *m_ctx;

  fun::FPtr<fpp::FDepsGraph> m_graph;
  fun::FPtr<FIDump>          m_dump;
  fun::FPtr<FIConstFolder>   m_constFolder;
  fun::FPtr<FILLVMCompiler>  m_llvmCompiler;

  ScheduleContext *m_sctx;

  void resolveScope();

public:
  constexpr auto &dump() const { return m_dump; }
  constexpr auto &constFolder() const { return m_constFolder; }
  constexpr auto &llvmCompiler() const { return m_llvmCompiler; }

  FIScheduler(const fun::FPtr<fpp::FDepsGraph> &graph,
              FIContext &                       ctx,
              const std::string &               moduleName) :
      m_ctx(&ctx),
      m_graph(graph),
      m_dump(fnew<FIDump>(ctx, std::cerr)),
      m_constFolder(fnew<FIConstFolder>(ctx)),
      m_llvmCompiler(fnew<FILLVMCompiler>(ctx, moduleName)) {}

  void scheduleGlobalConst(const std::string &,
                           fpp::FDepsNodeHelper<fie::FIConst *> &);

  void start();

  void finish();
};
} // namespace fie
