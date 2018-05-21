/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (driver.cpp)
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

#include "driver.hpp"

#include <iostream>

#include "intermedia/scheduler.hpp"

#include "scheduler.hpp"

namespace pre {
void FPDriver::run(RunMode mode, const fps::FInputs *inputs) {
  {
    fpp::FDepsGraph graph(m_ctx->logger());
    FPScheduler     sched(graph, *m_ctx);

    sched.schedule(inputs);

    if (mode == DotPreDeps) {
      graph.dot(std::cout);
      return;
    }

    graph.run();
  }

  {
    fpp::FDepsGraph  graph(m_ctx->logger());
    fie::FIScheduler sched(graph, m_ctx->fiCtx(), "cool module wow");

    sched.schedule();

    if (mode == DotFIDeps) {
      graph.dot(std::cout);
      return;
    }

    graph.run();
  }
}
} // namespace pre
