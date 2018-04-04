/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (function.cpp)
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

#include "function.hpp"

#include <iostream>
#include <queue>
#include <unordered_set>

#include "util/compilerError.hpp"

#include "scope.hpp"

namespace fie {
namespace pc {
  FuncClosure::FuncClosure(FIFunctionBody &body, const frma::FormaAST *curr) :
      PositionTracker(curr),
      m_args(fnew<ScopeClosure>(true, fun::weak(this), nullptr)),
      m_body(&body) {
    m_scope = m_args;
    pushScope();
  }

#if defined(DEBUG)
  FuncClosure::~FuncClosure() {
    if (!m_clean)
      std::cerr << "WARNING: FuncClosure::cleanup() not called." << std::endl;
  }
#endif

  void FuncClosure::pushScope() {
    m_scope = fnew<ScopeClosure>(false, fun::weak(this), m_scope);
  }

  void FuncClosure::dropScope() { m_scope = m_scope->parent(); }

  fun::FPtr<ScopeClosure> FuncClosure::popScope() {
    auto ret = std::move(m_scope);
    m_scope  = ret->parent();
    return ret;
  }

  void FuncClosure::cleanup() {
    // Find unreachable implicitly-generated "return void" blocks
    std::queue<fun::FPtr<FIBlock>>         q;
    std::unordered_set<fun::FPtr<FIBlock>> marked;

    q.push(m_body->blocks.at(0));

    while (q.size()) {
      auto &blk = q.front();

      if (marked.find(blk) == marked.end()) {
        marked.emplace(blk);

        switch (blk->cont()) {
        case FIBlock::BranchFT:
        case FIBlock::BranchTF:
          q.push(blk->contA().lock());
          q.push(blk->contB().lock());
          break;
        case FIBlock::Return: break;
        case FIBlock::Static: q.push(blk->contA().lock()); break;
        case FIBlock::ERR: assert(false);
        }
      }

      q.pop();
    }

    std::vector<decltype(m_body->blocks)::iterator> erase;

    for (auto it = m_body->blocks.begin(); it != m_body->blocks.end(); ++it) {
      auto &block = *it;
      if (marked.find(block) == marked.end() && block->body().size() == 1 &&
          m_implicitVoids.find(block->body().at(0).reg()) !=
              m_implicitVoids.end()) {
        erase.push_back(it);
      }
    }

    for (auto &block : erase) m_body->blocks.erase(block);

#if defined(DEBUG)
    m_clean = true;
#endif
  }
} // namespace pc
} // namespace fie
