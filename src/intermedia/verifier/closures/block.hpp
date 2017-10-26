/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (block.hpp)
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

#include <queue>
#include <stack>
#include <unordered_map>
#include <unordered_set>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "intermedia/assembly.hpp"
#include "intermedia/function.hpp"

namespace fie {
namespace vc {
  class BlockClosure : public fun::FObject {
    fun::FPtr<FIAssembly>                m_assem;
    fun::FPtr<const FIFunction>          m_func;
    std::queue<fun::FPtr<BlockClosure>> *m_q;
    std::size_t                          m_pc = 0;
    std::stack<std::uint32_t>            m_stack;
    std::unordered_map<std::uint32_t, std::uint32_t> m_vars;

    bool assertArity(std::uint32_t, const char *);

    void handlePHOp(std::uint32_t, fun::FPtr<FIStruct>, const char *);

    void handleBoolOp(std::uint32_t, const char *);

    bool handlePopBool(const char *);

  public:
    BlockClosure(fun::FPtr<FIAssembly>,
                 fun::FPtr<const FIFunction>,
                 std::queue<fun::FPtr<BlockClosure>> *);

    BlockClosure(fun::FPtr<BlockClosure>, std::size_t);

    void iterate();
  };
}
}
