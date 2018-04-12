/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ptr.hpp)
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

#include <cassert>
#include <functional>
#include <stdexcept>

#include "object/refTracker.hpp"
#include "util/hashing.hpp"

namespace fun {

template <typename T>
class FPtr;

template <typename T>
class FWeakPtr;
} // namespace fun

namespace std {
template <typename T>
struct hash<fun::FPtr<T>>;

template <typename T>
struct hash<fun::FWeakPtr<T>>;
} // namespace std

namespace fun {
template <typename, typename, typename...>
class FMFPtr;

template <typename T>
class FPtr {
  T *m_ptr = nullptr;

public:
  T *get() const { return m_ptr; }

  explicit FPtr(T *ptr) : m_ptr(ptr) {
    if (m_ptr) m_ptr->acquire();
  }

  FPtr(decltype(nullptr)) : FPtr() {} // Keep this implicit

  FPtr() : FPtr(static_cast<T *>(nullptr)) {}

  template <typename U>
  FPtr(const FPtr<U> &other) : FPtr(static_cast<T *>(other.m_ptr)) {}

  FPtr(const FPtr &other) : FPtr(other.m_ptr) {}

  FPtr(FPtr &other) : FPtr(const_cast<const FPtr &>(other)) {}

  FPtr(FPtr &&other) : m_ptr(other.m_ptr) { other.m_ptr = nullptr; }

  ~FPtr() {
    if (m_ptr) m_ptr->release();

    m_ptr = nullptr;
  }

  bool nil() const { return !m_ptr; }

  template <typename U>
  U *as() const {
    return dynamic_cast<U *>(m_ptr);
  }

  const FPtr &operator=(const FPtr &rhs) {
    if (&rhs == this) goto done;

    this->~FPtr();
    new (this) FPtr(rhs);
  done:
    return *this;
  }

  const FPtr &operator=(FPtr &rhs) {
    return operator=(const_cast<const FPtr &>(rhs));
  }

  const FPtr &operator=(FPtr &&rhs) {
    if (&rhs == this) goto done;

    this->~FPtr();
    new (this) FPtr(std::move(rhs));
  done:
    return *this;
  }

  T *operator->() const {
    assert(m_ptr);
    return m_ptr;
  }
  T &operator*() const {
    assert(m_ptr);
    return *m_ptr;
  }

  template <typename U>
  U operator->*(U T::*memb) const {
    assert(m_ptr);
    return m_ptr->*memb;
  }

  template <typename U, typename... TArgs>
  FMFPtr<T, U, TArgs...> operator->*(U (T::*memb)(TArgs...)) const {
    assert(m_ptr);
    return FMFPtr<T, U, TArgs...>(*this, memb);
  }

  bool operator==(const FPtr &rhs) const { return m_ptr == rhs.m_ptr; }
  bool operator!=(const FPtr &rhs) const { return m_ptr != rhs.m_ptr; }

  template <typename U>
  auto operator<<(U &&rhs) {
    return m_ptr->operator<<(std::forward<U>(rhs));
  }

  template <typename U>
  auto operator>>(U &&rhs) {
    return m_ptr->operator>>(std::forward<U>(rhs));
  }

  template <typename... TArgs>
  auto operator[](TArgs &&... args) {
    return m_ptr->operator[](std::forward<TArgs>(args)...);
  }

  friend struct std::hash<FPtr>;

  template <typename>
  friend class FPtr; // For access into other pointer types

  template <typename, typename, typename...>
  friend class FMFPtr;
};

template <typename T, typename U, typename... TArgs>
class FMFPtr {
  using PMF_t = U (T::*)(TArgs...);

  fun::FPtr<T> m_ptr;
  PMF_t        m_memb;

public:
  FMFPtr(fun::FPtr<T> ptr, PMF_t memb) : m_ptr(ptr), m_memb(memb) {}

  template <typename... UArgs>
  U operator()(UArgs &&... args) {
    return (m_ptr.m_ptr->*m_memb)(std::forward<TArgs>(args)...);
  }
};

template <typename T>
FPtr<T> wrap(T &&obj) {
  return FPtr<T>(std::forward<T>(obj));
}

template <typename T>
FPtr<T> wrap(const T &obj) {
  return FPtr<T>(&obj);
}

template <typename T>
FPtr<T> wrap(T &obj) {
  return FPtr<T>(&obj);
}

template <typename T>
FPtr<T> wrap(T *obj) {
  return FPtr<T>(obj);
}

template <typename T>
class FWeakPtr {
  FPtr<FRefTracker> m_ptr = nullptr;

public:
  explicit FWeakPtr(T *ptr) : m_ptr(ptr ? ptr->tracker() : nullptr) {}

  explicit FWeakPtr(FPtr<T> ptr) : FWeakPtr(ptr.get()) {}

  FWeakPtr(decltype(nullptr)) : FWeakPtr() {} // Keep this implicit.

  FWeakPtr() : FWeakPtr(static_cast<T *>(nullptr)) {}

  template <typename U>
  FWeakPtr(const FWeakPtr<U> &other) : m_ptr(other.m_ptr) {
    static_assert(std::is_convertible<U, T>::value,
                  "cannot convert pointer base type");
  }

  FWeakPtr(const FWeakPtr<T> &) = default;
  FWeakPtr(FWeakPtr<T> &)       = default;
  FWeakPtr(FWeakPtr<T> &&)      = default;

  T *peek() const { return reinterpret_cast<T *>(m_ptr->target()); }

  bool nil() const { return m_ptr.nil(); }

  bool good() const {
    if (m_ptr.nil()) return false;

    switch (m_ptr->refCount()) {
    case FRefTracker::COUNT_DESTROYING:
    case 0: return false;
    case FRefTracker::COUNT_UNCLAIMED:
    default: return true;
    }
  }

  FPtr<T> lock() const {
    if (m_ptr.nil()) return FPtr<T>();

    switch (m_ptr->trackedCount()) {
    case FRefTracker::COUNT_DESTROYING:
      throw std::runtime_error("pointer use during free");
    case 0: throw std::runtime_error("pointer use after free");
    case FRefTracker::COUNT_UNCLAIMED:
    default: return wrap(reinterpret_cast<T *>(m_ptr->target()));
    }
  }

  FPtr<T> lockOrNull() const {
    if (m_ptr.nil()) goto nil;

    switch (m_ptr->trackedCount()) {
    case FRefTracker::COUNT_DESTROYING:
      throw std::runtime_error("pointer use during free");
    case 0: goto nil;
    case FRefTracker::COUNT_UNCLAIMED:
    default: return wrap(reinterpret_cast<T *>(m_ptr->target()));
    }

  nil:
    return FPtr<T>();
  }

  const FWeakPtr &operator=(const FWeakPtr &rhs) {
    if (&rhs == this) goto done;

    m_ptr = rhs.m_ptr;

  done:
    return *this;
  }

  const FWeakPtr &operator=(FWeakPtr &rhs) {
    return operator=(const_cast<const FWeakPtr &>(rhs));
  }

  const FWeakPtr &operator=(FWeakPtr &&rhs) {
    if (&rhs == this) goto done;

    m_ptr = std::move(rhs.m_ptr);

  done:
    return *this;
  }

  bool operator==(const FWeakPtr &rhs) const { return m_ptr == rhs.m_ptr; }
  bool operator!=(const FWeakPtr &rhs) const { return m_ptr != rhs.m_ptr; }

  friend struct std::hash<FWeakPtr>;

  template <typename>
  friend class FWeakPtr;
};

template <typename T>
FWeakPtr<T> weak(const FPtr<T> &obj) {
  return FWeakPtr<T>(obj);
}

template <typename T>
FWeakPtr<T> weak(T *obj) {
  return FWeakPtr<T>(obj);
}
} // namespace fun

namespace std {
template <typename T>
struct hash<fun::FPtr<T>> {
public:
  std::size_t operator()(const fun::FPtr<T> &ptr) const {
    return hash<T *>{}(ptr.m_ptr);
  }
};

template <typename T>
struct hash<fun::FWeakPtr<T>> {
public:
  std::size_t operator()(const fun::FWeakPtr<T> &ptr) const {
    return fun::multiHash(ptr.m_ptr, fun::forward_hash(typeid(T).hash_code()));
  }
};
} // namespace std

template <typename T, typename... TArgs>
[[nodiscard]] fun::FPtr<T> fnew(TArgs &&... args) {
  return fun::FPtr<T>(new T(std::forward<TArgs>(args)...));
}
