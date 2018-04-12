/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (type.hpp)
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

#include "_type.hpp"
#include "types.hpp"

#include "util/ref.hpp"

namespace w {
template <typename T>
typename Type<T>::UnifyResult Type<T>::mguImpl(
    const fun::FPtr<const TypeBase> &rhs, TIBase &t) const {
  {
    auto type = rhs.as<const Type>();
    if (!(type && *m_base == *type->m_base)) return UnifyResult(false, Subst());

    Subst s;
    for (int i = 0; i < m_params.size(); ++i) {
      auto s2 = w::sub(s, m_params[i])->mgu(w::sub(s, type->m_params[i]), t);
      s       = composeSubst(s2, s);
    }

    return UnifyResult(true, s);
  }
}

template <typename T>
fun::FPtr<const TypeBase> Type<T>::sub(const Subst &s) const {
  Params params;

  for (auto &param : m_params) params.emplace_back(w::sub(s, param));

  return fnew<Type>(m_base, params);
}

template <typename T>
void Type<T>::hashImpl(std::size_t &seed) const {
  fun::combineHashes(seed, fun::ref(m_base));

  for (auto param : m_params) fun::combineHashes(seed, fun::ref(param));
}

template <typename T>
fun::FPtr<const TypeBase> CustomType<T>::sub(const Subst &s) const {
  Params params;

  for (auto &param : m_params) params.emplace_back(w::sub(s, param));

  return fnew<CustomType>(m_base, params);
}

template <typename T>
void CustomType<T>::hashImpl(std::size_t &seed) const {
  fun::combineHashes(seed, fun::ref(m_base));

  for (auto param : m_params) fun::combineHashes(seed, fun::ref(param));
}

template <>
struct Types<fun::FPtr<const TypeBase>> {
  static TypeVars __ftv(const fun::FPtr<const TypeBase> &);

  static fun::FPtr<const TypeBase> __sub(const Subst &,
                                         const fun::FPtr<const TypeBase> &);
};

template <>
struct Types<fun::FRef<const TypeBase>> {
  static TypeVars __ftv(const fun::FRef<const TypeBase> &);

  static fun::FRef<const TypeBase> __sub(const Subst &,
                                         const fun::FRef<const TypeBase> &);
};
} // namespace w
