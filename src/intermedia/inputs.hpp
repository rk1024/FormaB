/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (inputs.hpp)
* Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
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

#include <unordered_map>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "ast/astBase.hpp"

#include "assembly.hpp"

namespace fie {
class FIInputs : public fun::FObject {
  fun::FPtr<FIAssembly> m_assem;
  std::unordered_map<const frma::FormaAST *, std::uint32_t> m_funcs;
  std::unordered_map<const frma::FormaAST *, std::uint32_t> m_structs;

public:
  inline fun::FPtr<FIAssembly> assem() { return m_assem; }
  inline auto &                funcs() { return m_funcs; }
  inline auto &                structs() { return m_structs; }

  FIInputs(fun::FPtr<FIAssembly>);
};
}
