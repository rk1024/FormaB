/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scope.hpp)
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
#include <string>
#include <unordered_map>
#include <vector>

#include "util/cons.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "intermedia/atoms.hpp"

namespace fie {
namespace pc {
  class FuncClosure;

  class ScopeClosure : public fun::FObject {
  public:
    using VarInfo =
        fun::cons_cell<fun::FWeakPtr<ScopeClosure>, std::string, bool>;

    using OwnVarInfo = fun::cons_cell<std::string, bool>;

  private:
    static const unsigned int ID_NONE = 0xffffffff, ID_MAX = 0xfffffffe;
    static const unsigned int COUNT_CONST = 0xffffffff, COUNT_MAX = 0xfffffffe;

    fun::FWeakPtr<FuncClosure>            m_func;
    fun::FPtr<ScopeClosure>               m_parent;
    unsigned int                          m_id;
    bool                                  m_isArgs;
    std::unordered_map<std::string, bool> m_vars;

    std::string assembleName(const std::string &);

    template <bool>
    fun::FPtr<ScopeClosure> holderOf(const std::string &);

    FIVariableAtom recordName(const std::string &);

  public:
    inline fun::FPtr<ScopeClosure> parent() const { return m_parent; }

    ScopeClosure(bool, fun::FWeakPtr<FuncClosure>, fun::FPtr<ScopeClosure>);

    FIVariableAtom bind(const std::string &, bool mut);
    FIVariableAtom get(const std::string &);
    FIVariableAtom set(const std::string &);

    std::vector<OwnVarInfo> getOwned();
  };
} // namespace pc
} // namespace fie
