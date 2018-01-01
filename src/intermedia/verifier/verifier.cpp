/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (verifier.cpp)
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

#include "verifier.hpp"

#include <iostream>
#include <queue>
#include <stack>
#include <unordered_set>

#include "util/cons.hpp"

#include "closures/block.hpp"

using namespace fie::vc;

namespace fie {
FIVerifier::FIVerifier(fun::FPtr<FIInputs> inputs) : m_inputs(inputs) {}

void FIVerifier::verifyFunc(fun::cons_cell<std::uint32_t> args) {
  auto func = m_inputs->assem()->funcs().value(args.get<0>());
  std::queue<fun::FPtr<BlockClosure>> q;
  std::unordered_set<std::size_t>     checked;
  q.push(fnew<BlockClosure>(m_inputs->assem(), func, &q, &checked));

  while (q.size()) {
    q.front()->iterate();
    q.pop();
  }
}
} // namespace fie
