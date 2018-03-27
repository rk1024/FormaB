/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (linearPtr.hpp)
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

#include <utility>

#include "ptr.hpp"

namespace fun {
template <typename>
class FLinearObject;

template <typename T>
class FLinearPtr {
  T *m_ptr = nullptr;

public:
  inline T *get() const { return m_ptr; }

  template <typename U>
  inline U *as() const {
    return dynamic_cast<U *>(m_ptr);
  }

  FLinearPtr &&move() & {
    assert(m_ptr);
    return std::move(*this);
  }

  explicit FLinearPtr(T *ptr) : m_ptr(ptr) {
    if (m_ptr) m_ptr->own(this);
  }

  FLinearPtr(decltype(nullptr)) : FLinearPtr() {} // Keep this implicit

  FLinearPtr() : FLinearPtr(static_cast<T *>(nullptr)) {}

  template <typename U>
  FLinearPtr(FLinearPtr<U> &&other) :
      FLinearPtr(static_cast<T *>(other.m_ptr)) {}

  FLinearPtr(const FLinearPtr &) = delete;
  FLinearPtr(FLinearPtr &)       = delete;

  FLinearPtr(FLinearPtr &&other) : FLinearPtr(other.m_ptr) {}

  ~FLinearPtr() {
    if (m_ptr) delete m_ptr;

    m_ptr = nullptr;
  }

  const FLinearPtr &operator=(const FLinearPtr &) = delete;
  const FLinearPtr &operator=(FLinearPtr &) = delete;

  const FLinearPtr &operator=(FLinearPtr &&rhs) {
    if (&rhs == this) goto done;

    this->~FLinearPtr();
    new (this) FLinearPtr(rhs.move());
  done:
    return *this;
  }

  inline T *operator->() const {
    assert(m_ptr);
    return m_ptr;
  }

  inline T &operator*() const {
    assert(m_ptr);
    return *m_ptr;
  }

  // TODO: Maybe implement member pointers?

  inline bool operator==(const FLinearPtr &rhs) const {
    return m_ptr == rhs.m_ptr;
  }
  inline bool operator!=(const FLinearPtr &rhs) const {
    return m_ptr != rhs.m_ptr;
  }
  inline bool operator<(const FLinearPtr &rhs) const {
    return m_ptr < rhs.m_ptr;
  }
  inline bool operator>(const FLinearPtr &rhs) const {
    return m_ptr > rhs.m_ptr;
  }
  inline bool operator<=(const FLinearPtr &rhs) const {
    return m_ptr <= rhs.m_ptr;
  }
  inline bool operator>=(const FLinearPtr &rhs) const {
    return m_ptr >= rhs.m_ptr;
  }

  inline bool operator!() const { return !m_ptr; }
  inline      operator bool() const { return !!m_ptr; }

  template <typename U>
  inline auto operator<<(U &&rhs) {
    return m_ptr->operator<<(std::forward<U>(rhs));
  }

  template <typename U>
  inline auto operator>>(U &&rhs) {
    return m_ptr->operator>>(std::forward<U>(rhs));
  }

  template <typename... TArgs>
  inline auto operator[](TArgs &&... args) {
    return m_ptr->operator[](std::forward<TArgs>(args)...);
  }

  template <typename>
  friend class FLinearPtr;

  friend class FLinearObject<T>;
};

template <typename T>
FLinearPtr<T> wrapLinear(T &&obj) {
  return FLinearPtr<T>(std::forward<T>(obj));
}

template <typename T>
FLinearPtr<T> wrapLinear(const T &obj) {
  return FLinearPtr<T>(&obj);
}

template <typename T>
FLinearPtr<T> wrapLinear(T &obj) {
  return FLinearPtr<T>(&obj);
}

template <typename T>
FLinearPtr<T> wrapLinear(T *obj) {
  return FLinearPtr<T>(obj);
}
} // namespace fun

template <typename T, typename... TArgs>
[[nodiscard]] fun::FLinearPtr<T> flinear(TArgs &&... args) {
  return fun::FLinearPtr<T>(new T(std::forward<TArgs>(args)...));
}
