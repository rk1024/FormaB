/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (expr.hpp)
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

#include "_expr.hpp"

#include "ti.hpp"

namespace w {
template <typename T>
typename ExprBase<T>::TIResult ExprBase<T>::ti(TI<T> &t) const {
  TIPos _(t, "\e[1mti\e[0m " + to_string());
  auto  ret = tiImpl(t);

#if defined(DEBUG)
  TIPos __(t,
           "\e[38;5;2mresult:\e[0m " + to_string() +
               " :: " + ret.template get<2>()->to_string() +
               printConstraints(ret.template get<1>()) +
               printSubst(ret.template get<0>()));
#endif

  return ret;
}
} // namespace w
