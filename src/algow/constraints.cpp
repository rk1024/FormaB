/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (constraints.cpp)
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

#include "constraints.hpp"

#include <sstream>

#include "util/gsub.hpp"

#include "type.hpp"
#include "types.hpp"

namespace w {
fun::FPtr<const ConstraintBase> ConstraintBase::sub(const Subst &s) const {
  auto ret = subImpl(s);

  if (!validate())
    throw std::runtime_error("constraint substitution failed: " + to_string() +
                             printSubst(s));

  return ret;
}

TypeVars DummyConstraint::ftv() const { return {}; }

fun::FPtr<const ConstraintBase> DummyConstraint::subImpl(const Subst &) const {
  return fnew<DummyConstraint>();
}

bool DummyConstraint::validate() const { return true; }

bool DummyConstraint::includes(const TypeVars &) const { return true; }

std::string DummyConstraint::to_string() const { return "<dummy constraint>"; }

std::size_t DummyConstraint::hash() const { return 99999; }

bool DummyConstraint::operator==(const ConstraintBase &rhs) const {
  auto dummy = dynamic_cast<const DummyConstraint *>(&rhs);
  return dummy;
}

Constraints mergeConstraints(const Constraints &c1, const Constraints &c2) {
  Constraints ret = c1;

  ret.insert(c2.begin(), c2.end());

  return ret;
}

std::string printConstraints(const Constraints &c) {
  std::ostringstream oss;

  for (auto &constraint : c)
    oss << "\n  where " + fun::gsub(constraint->to_string(), "\n", "\n  ");

  return oss.str();
}

TypeVars Types<fun::FPtr<const ConstraintBase>>::__ftv(
    const fun::FPtr<const ConstraintBase> &c) {
  return c->ftv();
}

fun::FPtr<const ConstraintBase> Types<fun::FPtr<const ConstraintBase>>::__sub(
    const Subst &s, const fun::FPtr<const ConstraintBase> &c) {
  return c->sub(s);
}

TypeVars Types<fun::FRef<const ConstraintBase>>::__ftv(
    const fun::FRef<const ConstraintBase> &c) {
  return c->ftv();
}

fun::FRef<const ConstraintBase> Types<fun::FRef<const ConstraintBase>>::__sub(
    const Subst &s, const fun::FRef<const ConstraintBase> &c) {
  return c->sub(s);
}

TypeVars Types<Constraints>::__ftv(const Constraints &c) {
  TypeVars ret;

  for (auto &constraint : c) {
    auto vars = ftv(constraint);

    ret.insert(vars.begin(), vars.end());
  }

  return ret;
}

Constraints Types<Constraints>::__sub(const Subst &s, const Constraints &c) {
  Constraints ret;

  for (auto &constraint : c) ret.emplace(sub(s, constraint));

  return ret;
}
} // namespace w
