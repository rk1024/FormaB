/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scope.cpp)
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

#include "scope.hpp"

#include <sstream>

#include "function.hpp"

namespace fie {
namespace pc {
  std::string ScopeClosure::assembleName(const std::string &name) {
    assert(m_isArgs || m_id != ID_NONE);

    std::ostringstream oss;
    if (m_isArgs)
      oss << 'a';
    else
      oss << m_id;
    oss << '@' << name;
    return oss.str();
  }

  template <bool required>
  fun::FPtr<ScopeClosure> ScopeClosure::holderOf(const std::string &name) {
    if (m_vars.find(name) != m_vars.end()) return fun::wrap(this);
    if (!m_parent.nil()) return m_parent->holderOf<required>(name);
    if constexpr (required)
      m_func.lock()->error("variable '" + name + "' not declared");
    else
      return nullptr;
  }

  template fun::FPtr<ScopeClosure> ScopeClosure::holderOf<false>(
      const std::string &name);

  template fun::FPtr<ScopeClosure> ScopeClosure::holderOf<true>(
      const std::string &name);

  FIVariableAtom ScopeClosure::recordName(const std::string &name) {
    auto func = m_func.lock();

    if (!m_isArgs && m_id == ID_NONE) {
      m_id = func->m_nextScopeId;
      ++func->m_nextScopeId;

      if (m_id > ID_MAX) func->error("scope ID limit exceeded");
    }

    return func->m_body->vars.intern(assembleName(name));
  }

  ScopeClosure::ScopeClosure(bool                       isArgs,
                             fun::FWeakPtr<FuncClosure> func_,
                             fun::FPtr<ScopeClosure>    parent) :
      m_func(func_),
      m_parent(parent),
      m_id(ID_NONE),
      m_isArgs(isArgs) {}

  FIVariableAtom ScopeClosure::bind(const std::string &name, bool mut) {
    auto holder = holderOf<false>(name);

    if (holder.get() == this)
      m_func.lock()->error("variable '" + name +
                           "' already declared in this scope");
    else if (!holder.nil())
      // TODO: add proper diagnostic logging
      std::cerr << "WARNING: variable '" + name + "' shadows outer scope";

    m_vars[name] = mut;
    return recordName(name);
  }

  FIVariableAtom ScopeClosure::get(const std::string &name) {
    auto holder = holderOf<true>(name);

    return holder->recordName(name);
  }

  FIVariableAtom ScopeClosure::set(const std::string &name) {
    auto holder = holderOf<true>(name);
    auto mut    = holder->m_vars.at(name);

    if (!mut) m_func.lock()->error("variable '" + name + "' is immutable");

    return holder->recordName(name);
  }

  std::vector<ScopeClosure::OwnVarInfo> ScopeClosure::getOwned() {
    std::vector<OwnVarInfo> owned;

    for (auto pair : m_vars)
      owned.push_back(fun::cons(pair.first, pair.second));

    return owned;
  }
} // namespace pc
} // namespace fie
