/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (typeSolver.hpp)
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

#include "util/object/object.hpp"

#include "intermedia/inputs.hpp"

#include "algow/scheme.hpp"

namespace fie {
class FITypeSolver : public fun::FObject {
  fun::FPtr<FIInputs> m_inputs;

  std::unordered_map<fun::FPtr<const FIFunction>, w::Scheme> m_funcs;
  std::unordered_map<FIMessage, w::Scheme>                   m_msgs;
  std::unordered_map<FIMessage, std::unordered_set<fun::FRef<w::TypeBase>>>
      m_accepts;

  void setupMsgs();

public:
  FITypeSolver(const fun::FPtr<FIInputs> &inputs) : m_inputs(inputs) {
    setupMsgs();
  }

  void typeFunc(fun::cons_cell<FIFunctionAtom>);

  friend class TIContext;
};
} // namespace fie
