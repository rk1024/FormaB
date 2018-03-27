/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ti.hpp)
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

#include "algow/ti.hpp"
#include "algow/type.hpp"

#include "intermedia/function/values.hpp"
#include "intermedia/inputs.hpp"
#include "intermedia/typeSolver/typeSolver.hpp"

namespace fie {
class TIContext {
  std::unordered_map<const FIValue *, fun::FPtr<const w::TypeBase>> types;

public:
  const fun::FPtr<const FIInputs>                              inputs;
  const fun::FPtr<const FITypeSolver>                          solver;
  const std::unordered_map<FIRegId, fun::FPtr<const FIValue>> *values;
  w::TypeEnv<FIVariableAtom>                                   env;
  w::Subst                                                     subst;

  auto &solverFuncs() const { return solver->m_funcs; }
  auto &solverMsgs() const { return solver->m_msgs; }

  auto getType(const FIRegId &reg) const {
    return w::sub(subst, types.at(values->at(reg).get()));
  }

  void putType(const FIRegId &reg, const fun::FPtr<const w::TypeBase> &tp) {
    auto val = values->at(reg).get();
    assert(types.find(val) == types.end());
    types.emplace(val, tp);
  }

  auto getEnv(const FIVariableAtom &var) const {
    return w::sub(subst, env.at(var));
  }

  TIContext(decltype(inputs) & _inputs,
            decltype(solver) & _solver,
            decltype(*values) &_values) :
      inputs(_inputs),
      solver(_solver),
      values(&_values) {}
};

using TI = w::TI<TIContext>;
} // namespace fie
