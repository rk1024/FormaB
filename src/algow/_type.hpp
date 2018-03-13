/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (_type.hpp)
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

#include <iostream>
#include <sstream>
#include <unordered_set>
#include <utility>

#include "util/consPod.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "subst.hpp"

namespace w {
struct TI;

class TypeBase : public fun::FObject {
public:
  using TypeVars    = std::unordered_set<std::string>;
  using UnifyResult = std::pair<bool, Subst>;

protected:
  virtual UnifyResult mguImpl(const fun::FPtr<const TypeBase> &,
                              TI &) const = 0;

  virtual UnifyResult rmguImpl(const fun::FPtr<const TypeBase> &, TI &) const;

public:
  virtual TypeVars ftv() const = 0;

  virtual fun::FPtr<const TypeBase> sub(const Subst &) const = 0;

  Subst mgu(const fun::FPtr<const TypeBase> &, TI &) const;

  virtual std::string to_string() const = 0;

  virtual bool operator==(const TypeBase &) const = 0;

  constexpr bool operator!=(const TypeBase &rhs) const {
    return !operator==(rhs);
  }
};

class TypeVar : public TypeBase {
  std::string m_var;

protected:
  virtual UnifyResult mguImpl(const fun::FPtr<const TypeBase> &,
                              TI &) const override;

public:
  constexpr auto &var() const { return m_var; }

  TypeVar(const std::string &var) : m_var(var) {}

  virtual TypeVars ftv() const override;

  virtual fun::FPtr<const TypeBase> sub(const Subst &) const override;

  virtual std::string to_string() const override;

  virtual bool operator==(const TypeBase &) const override;
};

template <typename T>
class Type : public TypeBase {
public:
  using Params = std::vector<fun::FPtr<const TypeBase>>;

private:
  fun::FPtr<const T> m_base;
  Params             m_params;

protected:
  virtual UnifyResult mguImpl(const fun::FPtr<const TypeBase> &rhs,
                              TI &t) const override {
    auto type = rhs.as<const Type>();
    if (!(type && *m_base == *type->m_base)) return UnifyResult(false, Subst());

    Subst s;
    for (int i = 0; i < m_params.size(); ++i) {
      auto s2 = m_params[i]->sub(s)->mgu(type->m_params[i]->sub(s), t);
      s       = composeSubst(s, s2);
    }

    return UnifyResult(true, s);
  }

public:
  constexpr auto &base() const { return m_base; }

  constexpr auto &params() const { return m_params; }

  Type(const fun::FPtr<const T> &base, const Params &params) :
      m_base(base),
      m_params(params) {
    assert(base->arity() == params.size());
  }

  virtual TypeVars ftv() const override {
    TypeVars ret;

    for (auto param : m_params) {
      auto vars = param->ftv();
      ret.insert(vars.begin(), vars.end());
    }

    return ret;
  }

  virtual fun::FPtr<const TypeBase> sub(const Subst &s) const override {
    Params params;

    for (auto &param : m_params) params.emplace_back(param->sub(s));

    return fnew<Type>(m_base, params);
  }

  virtual std::string to_string() const override {
    std::ostringstream oss;

    oss << m_base->to_string();

    if (m_params.size()) {
      oss << "[";

      bool first = true;
      for (auto &param : m_params) {
        if (first)
          first = false;
        else
          oss << ", ";

        oss << param->to_string();
      }

      oss << "]";
    }

    return oss.str();
  }

  virtual bool operator==(const TypeBase &rhs) const override {
    auto type = dynamic_cast<const Type *>(&rhs);
    if (!(type && *m_base == *type->m_base)) return false;

    assert(m_params.size() == type->m_params.size());

    for (int i = 0; i < m_params.size(); ++i) {
      if (m_params[i] != type->m_params[i]) return false;
    }

    return true;
  }
};

FUN_CONSPOD(ToyT, std::string, int), public fun::FObject {
  FCP_GET(0, name);
  FCP_GET(1, arity);

  ToyT(const std::string &_name, int _arity) : FCP_INIT(_name, _arity) {}

  std::string to_string() const { return name(); }
};

using TypeToy = Type<ToyT>;
} // namespace w
