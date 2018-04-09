/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (_expr.hpp)
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
#include "util/ptr.hpp"

#include "subst.hpp"
#include "typeEnv.hpp"

namespace w {
template <typename>
struct TI;
class TypeBase;

template <typename T>
class ExprBase : public fun::FObject {
public:
  using TIResult =
      fun::cons_cell<Subst, Constraints, fun::FPtr<const TypeBase>>;

protected:
  virtual TIResult tiImpl(TI<T> &) const = 0;

public:
  TIResult ti(TI<T> &) const;

  virtual std::string to_string() const = 0;
};
} // namespace w
