/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (verifier.hpp)
* Copyright (C) 2017 Ryan Schroeder, Colin Unger
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
*************************************************************************/

#pragma once

#include "util/cons.hpp"
#include "util/ptr.hpp"

#include "intermedia/inputs.hpp"

namespace fie {
class FIVerifier : public fun::FObject {
  fun::FPtr<FIInputs> m_inputs;

public:
  FIVerifier(fun::FPtr<FIInputs>);

  void verifyFunc(fun::cons_cell<std::uint32_t>);
};
}
