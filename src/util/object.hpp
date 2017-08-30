#pragma once

#include <cassert>
#include <functional>
#include <stdexcept>

#include "util/object/object.hpp"

namespace fun {
template <typename T>
class FPtr;

template <typename T>
class FWeakPtr;
}

namespace std {
template <typename T>
struct hash<fun::FPtr<T>>;

template <typename T>
struct hash<fun::FWeakPtr<T>>;
}

namespace fun {
template <typename T>
class FPtr {
  T *m_ptr;

public:
  T *      get() { return m_ptr; }
  const T *get() const { return m_ptr; }

  FPtr(T *ptr) : m_ptr(ptr) {
    if (m_ptr) m_ptr->acquire();
  }

  FPtr() : FPtr(nullptr) {}

  FPtr(const FPtr &other) : FPtr(other.m_ptr) {}
  FPtr(FPtr &other) : FPtr(const_cast<const FPtr &>(other)) {}
  FPtr(FPtr &&other) : FPtr(other.m_ptr) { other.~FPtr(); }

  ~FPtr() {
    if (m_ptr) m_ptr->release();
    m_ptr = nullptr;
  }

  const FPtr &operator=(const FPtr &rhs) {
    if (&rhs == this) return *this;

    this->~FPtr();
    new (this) FPtr(rhs);
    return *this;
  }

  const FPtr &operator=(FPtr &rhs) {
    return operator=(const_cast<const FPtr &>(rhs));
  }

  const FPtr &operator=(FPtr &&rhs) {
    if (&rhs == this) return *this;

    this->~FPtr();
    new (this) FPtr(std::forward<FPtr>(rhs));
    return *this;
  }

  T *operator->() { return m_ptr; }
  T *operator*() { return m_ptr; }
  const T *operator->() const { return m_ptr; }
  const T *operator*() const { return m_ptr; }

  bool operator==(const FPtr &rhs) const { return m_ptr == rhs.m_ptr; }
  bool operator!=(const FPtr &rhs) const { return m_ptr != rhs.m_ptr; }
  bool operator<(const FPtr &rhs) const { return m_ptr < rhs.m_ptr; }
  bool operator>(const FPtr &rhs) const { return m_ptr > rhs.m_ptr; }
  bool operator<=(const FPtr &rhs) const { return m_ptr <= rhs.m_ptr; }
  bool operator>=(const FPtr &rhs) const { return m_ptr >= rhs.m_ptr; }

  bool operator!() const { return !m_ptr; }
  operator bool() const { return !!m_ptr; }

  friend struct std::hash<FPtr>;
};

template <typename T>
FPtr<T> wrap(T &&obj) {
  return FPtr<T>(obj);
}

template <typename T>
FPtr<T> wrap(const T &obj) {
  return FPtr<T>(obj);
}

template <typename T>
FPtr<T> wrap(T *obj) {
  return FPtr<T>(obj);
}

template <typename T>
class FWeakPtr {
  FPtr<FRefTracker> m_ptr;

public:
  FWeakPtr(T *ptr) {
    if (ptr) {
      m_ptr = wrap(ptr->tracker());
      m_ptr->acquire();
    }
  }

  FWeakPtr(FPtr<T> ptr) : FWeakPtr(ptr.get()) {}

  T *peek() const { return reinterpret_cast<T *>(m_ptr->target()); }

  FPtr<T> lock() const {
    if (!m_ptr->live()) throw std::runtime_error("pointer use after release");
    return wrap(reinterpret_cast<T *>(m_ptr->target()));
  }

  bool operator!() const { return !(m_ptr && m_ptr->live()); }
  operator bool() const { return m_ptr && m_ptr->live(); }

  friend struct std::hash<FWeakPtr>;
};

template <typename T>
FWeakPtr<T> weak(const FPtr<T> &obj) {
  return FWeakPtr<T>(obj);
}
}

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
  std::size_t operator()(const fun::FPtr<T> &ptr) const {
    return hash<T *>{}(ptr.m_ptr);
  }
};
}

template <typename T, typename... TArgs>
fun::FPtr<T> fnew(TArgs &&... args) {
  return fun::FPtr<T>(new T(std::forward<TArgs>(args)...));
}
