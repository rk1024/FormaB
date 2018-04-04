/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ref.hpp)
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

#include "ptr.hpp"

namespace fun {
template <typename T>
class FRef;

template <typename T>
struct unwrap_ref {
  using type = T;

  static constexpr T &get(const T &val) { return val; }
};

template <typename T>
struct unwrap_ref<FRef<T>> {
  using type = T;

  static constexpr T &get(const FRef<T> &val) { return *val; }
};

template <typename T>
class FRef {
public:
  using FPtr = FPtr<T>;

private:
  FPtr m_ptr;

public:
  inline T *get() const { return m_ptr.get(); }

  explicit FRef(T *ptr) : m_ptr(ptr) {}

  FRef(const FPtr &ptr) : m_ptr(ptr) {}

  FRef(FPtr &ptr) : m_ptr(ptr) {}

  FRef(FPtr &&ptr) : m_ptr(std::move(ptr)) {}

  FRef(decltype(nullptr)) : FRef() {}

  FRef() : FRef(static_cast<T *>(nullptr)) {}

  template <typename U>
  FRef(const FRef<U> &other) : FRef(other.m_ptr) {}

  bool nil() const { return !m_ptr; }

  template <typename U>
  FRef<U> as() const {
    return FRef<U>(m_ptr.template as<U>());
  }

  operator T &() const { return *m_ptr; }

  T *operator->() const { return m_ptr.operator->(); }

  T &operator*() const { return m_ptr.operator*(); }

  template <typename U>
  U &operator->*(U T::*memb) const {
    return m_ptr.template operator->*<U>(memb);
  }

  template <typename U, typename... TArgs>
  FMFPtr<T, U, TArgs...> operator->*(U (T::*memb)(TArgs...)) const {
    return m_ptr.template operator->*<U, TArgs...>(memb);
  }

  template <typename... TArgs>
  decltype(auto) operator()(TArgs &&... args) const {
    return m_ptr->operator()(std::forward<TArgs>(args)...);
  }

#define _FREF_DEFER_BINOP(op)                                                  \
  template <typename U>                                                        \
  decltype(auto) operator op(U &&rhs) const {                                  \
    return m_ptr->operator op(std::forward<U>(rhs));                           \
  }

#define _FREF_DEFER_PREOP(op)                                                  \
  template <typename U>                                                        \
  decltype(auto) operator op() const {                                         \
    return m_ptr->operator op();                                               \
  }

#define _FREF_DEFER_POSTOP(op)                                                 \
  template <typename U>                                                        \
  decltype(auto) operator op(int) const {                                      \
    return m_ptr->operator op();                                               \
  }

  _FREF_DEFER_BINOP(+)
  _FREF_DEFER_BINOP(-)
  _FREF_DEFER_BINOP(*)
  _FREF_DEFER_BINOP(/)
  _FREF_DEFER_BINOP(%)
  _FREF_DEFER_BINOP(&)
  _FREF_DEFER_BINOP(|)
  _FREF_DEFER_BINOP (^)
  _FREF_DEFER_BINOP(<<)
  _FREF_DEFER_BINOP(>>)

  _FREF_DEFER_BINOP(&&)
  _FREF_DEFER_BINOP(||)

  _FREF_DEFER_BINOP(==)
  _FREF_DEFER_BINOP(!=)
  _FREF_DEFER_BINOP(<)
  _FREF_DEFER_BINOP(>)
  _FREF_DEFER_BINOP(<=)
  _FREF_DEFER_BINOP(>=)

  _FREF_DEFER_BINOP([]) // Technically not a binary operator, but this works

  _FREF_DEFER_PREOP(+)
  _FREF_DEFER_PREOP(-)
  _FREF_DEFER_PREOP(++)
  _FREF_DEFER_PREOP(--)
  _FREF_DEFER_PREOP(~)

  _FREF_DEFER_PREOP(!)

  _FREF_DEFER_POSTOP(++)
  _FREF_DEFER_POSTOP(--)

#undef _FREF_DEFER_BINOP
#undef _FREF_DEFER_PREOP
#undef _FREF_DEFER_POSTOP

  template <typename>
  friend class FRef;

  friend struct std::hash<fun::FRef<T>>;
};

template <typename T>
FRef<T> ref(const FPtr<T> &ptr) {
  return ptr;
}
} // namespace fun

namespace std {
template <typename T>
struct hash<fun::FRef<T>> {
private:
  hash<std::decay_t<T>> m_hash;

public:
  size_t operator()(const fun::FRef<T> &ref) const { return m_hash(*ref); }
};
} // namespace std

template <typename T, typename... TArgs>
[[nodiscard]] fun::FRef<T> fref(TArgs &&... args) {
  return fun::ref(fnew<T, TArgs...>(std::forward<TArgs>(args)...));
}
