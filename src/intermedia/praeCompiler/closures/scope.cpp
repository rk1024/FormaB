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
    if (m_parent) return m_parent->holderOf<required>(name);
    if (required)
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

  FIVariableAtom ScopeClosure::recordVar(fun::FWeakPtr<ScopeClosure> scope_,
                                         const std::string &         name,
                                         bool                        mut) {
    auto func  = m_func.lock();
    auto scope = scope_.lock();

    m_vars[name] = mut;

    if (scope.get() != this) m_borrowed[name] = scope_;

    return recordName(name);
  }

  ScopeClosure::ScopeClosure(bool                       isArgs,
                             fun::FWeakPtr<FuncClosure> func_,
                             fun::FPtr<ScopeClosure>    parent) :
      m_func(func_),
      m_parent(parent),
      m_id(ID_NONE),
      m_isArgs(isArgs) {}

  FIVariableAtom ScopeClosure::bind(const std::string &name, bool mut) {
    return recordVar(fun::weak(this), name, mut);
  }

  FIVariableAtom ScopeClosure::get(const std::string &name) {
    auto holder = holderOf<true>(name);

    return holder->recordName(name);
  }

  FIVariableAtom ScopeClosure::set(const std::string &name) {
    auto holder = holderOf<true>(name);
    auto mut    = holder->m_vars.at(name);

    if (!mut) m_func.lock()->error("variable '" + name + "' is immutable");

    return recordVar(fun::weak(holder), name, true);
  }

  std::vector<ScopeClosure::VarInfo> ScopeClosure::getModified() {
    std::vector<VarInfo> modified;

    for (auto pair : m_vars) {
      auto it = m_borrowed.find(pair.first);

      if (it != m_borrowed.end())
        modified.push_back(fun::cons(it->second, pair.first, pair.second));
    }

    return modified;
  }

  std::vector<ScopeClosure::OwnVarInfo> ScopeClosure::getOwned() {
    std::vector<OwnVarInfo> owned;

    for (auto pair : m_vars) {
      if (m_borrowed.find(pair.first) == m_borrowed.end())
        owned.push_back(fun::cons(pair.first, pair.second));
    }

    return owned;
  }
} // namespace pc
} // namespace fie
