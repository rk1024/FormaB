/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (function.hpp)
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

#include <cstdint>
#include <vector>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "instructions.hpp"
#include "intermedia/types/struct.hpp"
#include "label.hpp"
#include "variable.hpp"

namespace fie {
struct FIFunctionBody {
  std::vector<fun::FPtr<FIInstructionBase>>  instructions;
  fun::FAtomStore<FILabel, std::uint32_t>    labels;
  fun::FAtomStore<FIVariable, std::uint32_t> vars;
};

class FIFunction : public fun::FObject {
  std::unordered_map<FIVariableAtom, FIStructAtom> m_args;
  FIFunctionBody                                   m_body;

public:
  const auto &args() const { return m_args; }
  const auto &body() const { return m_body; }

  FIFunction(std::unordered_map<FIVariableAtom, FIStructAtom>,
             const FIFunctionBody &);
};
} // namespace fie
