/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (type.cpp)
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

#include "type.hpp"

#include <sstream>

#include "ti.hpp"

namespace w {
Subst TypeBase::mgu(const fun::FPtr<const TypeBase> &rhs, TIBase &t) const {
  {
    TIPos _(t, "\x1b[1mlmgu\x1b[m " + to_string() + " <-> " + rhs->to_string());
    auto [success, subst] = mguImpl(rhs, t);
    if (success) {
      TIPos __(t,
               "\x1b[38;5;2mresult:\x1b[m " + to_string() + " ~ " +
                   rhs->to_string() + printSubst(subst));
      return subst;
    }
  }

  {
    TIPos _(t, "\x1b[1mrmgu\x1b[m " + rhs->to_string() + " <-> " + to_string());
    auto [success, subst] = rhs->rmguImpl(fun::wrap(this), t);
    if (success) {
      TIPos __(t,
               "\x1b[38;5;2mresult:\x1b[m " + to_string() + " ~ " +
                   rhs->to_string() + printSubst(subst));
      return subst;
    }
  }

  throw std::runtime_error("failed to unify " + to_string() + " with " +
                           rhs->to_string() + "\n" + t.state());
}

TypeBase::UnifyResult TypeBase::rmguImpl(const fun::FPtr<const TypeBase> &rhs,
                                         TIBase &t) const {
  return mguImpl(rhs, t); // Assuming transitivity unless otherwise stated
}

TypeBase::UnifyResult TypeVar::mguImpl(const fun::FPtr<const TypeBase> &rhs,
                                       TIBase &t) const {
  if (*this == *rhs)
    return UnifyResult(true, Subst());
  else {
    auto vars = w::ftv(rhs);
    if (vars.find(m_var) != vars.end())
      throw std::runtime_error("unsolvable binding " + m_var + " ~ " +
                               rhs->to_string() + " (" + m_var +
                               " occurs on both sides)\n" + t.state());

    Subst subst;
    subst[m_var] = rhs;
    return UnifyResult(true, subst);
  }
}

TypeVars TypeVar::ftv() const {
  TypeVars ret;
  ret.emplace(m_var);
  return ret;
}

fun::FPtr<const TypeBase> TypeVar::sub(const Subst &s) const {
  auto it = s.find(m_var);
  if (it == s.end())
    return fun::wrap(this);
  else
    return it->second;
}

std::string TypeVar::to_string() const { return m_var; }

void TypeVar::hashImpl(std::size_t &seed) const {
  fun::combineHashes(seed, m_var);
}

bool TypeVar::operator==(const TypeBase &rhs) const {
  auto var = dynamic_cast<const TypeVar *>(&rhs);
  if (!var) return false;
  return m_var == var->m_var;
}

TypeVars Types<fun::FPtr<const TypeBase>>::__ftv(
    const fun::FPtr<const TypeBase> &t) {
  return t->ftv();
}

fun::FPtr<const TypeBase> Types<fun::FPtr<const TypeBase>>::__sub(
    const Subst &s, const fun::FPtr<const TypeBase> &t) {
  return t->sub(s);
}

TypeVars Types<fun::FRef<const TypeBase>>::__ftv(
    const fun::FRef<const TypeBase> &t) {
  return t->ftv();
}

fun::FRef<const TypeBase> Types<fun::FRef<const TypeBase>>::__sub(
    const Subst &s, const fun::FRef<const TypeBase> &t) {
  return t->sub(s);
}
} // namespace w
