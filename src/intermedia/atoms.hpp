/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (atoms.hpp)
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

#include "util/atom.hpp"
#include "util/ptr.hpp"

namespace fie {
class FIFunction;
struct FILabel;
struct FIMessage;
struct FIMessageKeyword;
class FIStruct;
struct FIVariable;

using FIFunctionAtom = fun::FAtom<std::uint32_t, fun::FPtr<const FIFunction>>;
using FILabelAtom    = fun::FAtom<std::uint32_t, FILabel>;
using FIMessageAtom  = fun::FAtom<std::uint32_t, FIMessage>;
using FIMessageKeywordAtom = fun::FAtom<std::uint32_t, FIMessageKeyword>;
using FIStringAtom         = fun::FAtom<std::uint32_t, std::string>;
using FIStructAtom         = fun::FAtom<std::uint32_t, fun::FPtr<FIStruct>>;
using FIVariableAtom       = fun::FAtom<std::uint32_t, FIVariable>;
} // namespace fie
