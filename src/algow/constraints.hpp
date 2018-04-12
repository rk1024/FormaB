/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (constraints.hpp)
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

#include <unordered_map>
#include <unordered_set>

#include "util/object/object.hpp"
#include "util/ref.hpp"

#include "_type.hpp"
#include "types.hpp"

namespace w {
class ConstraintBase;
}

namespace std {
template <>
struct hash<w::ConstraintBase>;
} // namespace std

namespace w {
struct TIBase;
class TypeBase;

class ConstraintBase : public fun::FObject {
protected:
public:
  virtual TypeVars ftv() const = 0;

  fun::FPtr<const ConstraintBase> sub(const Subst &) const;

  virtual fun::FPtr<const ConstraintBase> subImpl(const Subst &) const = 0;

  virtual bool validate() const = 0;

  virtual bool includes(const TypeVars &) const = 0;

  virtual std::string to_string() const = 0;

  virtual std::size_t hash() const = 0;

  virtual bool operator==(const ConstraintBase &) const = 0;

  bool operator!=(const ConstraintBase &rhs) const { return !operator==(rhs); }
};

class DummyConstraint : public ConstraintBase {
public:
  virtual TypeVars ftv() const override;

  virtual fun::FPtr<const ConstraintBase> subImpl(const Subst &) const override;

  virtual bool validate() const override;

  virtual bool includes(const TypeVars &) const override;

  virtual std::string to_string() const override;

  virtual std::size_t hash() const override;

  virtual bool operator==(const ConstraintBase &) const override;
};

template <typename T>
class IncludesConstraint : public ConstraintBase {
  fun::FRef<const TypeBase> m_type;
  T                         m_set;

public:
  IncludesConstraint(const fun::FRef<const TypeBase> &type, const T &set) :
      m_type(type),
      m_set(set) {}

  virtual TypeVars ftv() const override {
    auto vars  = w::ftv(m_type);
    auto vars2 = w::ftv(m_set);
    vars.insert(vars2.begin(), vars2.end());

    return vars;
  }

  virtual fun::FPtr<const ConstraintBase> subImpl(
      const Subst &s) const override {
    return fnew<IncludesConstraint>(w::sub(s, m_type), w::sub(s, m_set));
  }

  virtual bool validate() const override { return m_set.validate(m_type); }

  virtual bool includes(const TypeVars &vars) const override {
    return m_set.includes(vars);
  }

  virtual std::string to_string() const override {
    return m_type->to_string() + " ∈ " + m_set.to_string();
  }

  virtual std::size_t hash() const override {
    std::size_t seed = 0;
    fun::combineHashes(seed, m_type, m_set);
    return seed;
  }

  virtual bool operator==(const ConstraintBase &rhs) const override {
    auto includes = dynamic_cast<const IncludesConstraint *>(&rhs);
    return includes && m_type == includes->m_type && m_set == includes->m_set;
  }
};

using Constraints = std::unordered_set<fun::FRef<const ConstraintBase>>;

Constraints mergeConstraints(const Constraints &, const Constraints &);

std::string printConstraints(const Constraints &);

template <>
struct Types<fun::FPtr<const ConstraintBase>> {
  static TypeVars __ftv(const fun::FPtr<const ConstraintBase> &);

  static fun::FPtr<const ConstraintBase> __sub(
      const Subst &, const fun::FPtr<const ConstraintBase> &);
};

template <>
struct Types<fun::FRef<const ConstraintBase>> {
  static TypeVars __ftv(const fun::FRef<const ConstraintBase> &);

  static fun::FRef<const ConstraintBase> __sub(
      const Subst &, const fun::FRef<const ConstraintBase> &);
};

template <>
struct Types<Constraints> {
  static TypeVars __ftv(const Constraints &);

  static Constraints __sub(const Subst &, const Constraints &);
};
} // namespace w

namespace std {
template <>
struct hash<w::ConstraintBase> {
  size_t operator()(const w::ConstraintBase &c) const { return c.hash(); }
};
} // namespace std
