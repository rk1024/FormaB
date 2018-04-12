/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scheme.hpp)
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

#include "util/consPod.hpp"
#include "util/ptr.hpp"

#include "_type.hpp"
#include "constraints.hpp"
#include "types.hpp"

namespace w {
FUN_CONSPOD(Scheme,
            std::vector<std::string>,
            fun::FPtr<const TypeBase>,
            Constraints) {
  FCP_GET(0, vars);
  FCP_GET(1, type);
  FCP_GET(2, constraints);

  Scheme() = default;

  Scheme(const std::vector<std::string> & _vars,
         const fun::FPtr<const TypeBase> &_type,
         const Constraints &              _constraints) :
      FCP_INIT(Scheme, _vars, _type, _constraints) {}

  std::string to_string() const;
};

template <>
struct Types<Scheme> {
  static TypeVars __ftv(const Scheme &);

  static Scheme __sub(const Subst &, const Scheme &);
};
} // namespace w
