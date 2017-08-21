#pragma once

#include <memory>

namespace fun {
template <typename T>
class EnableRefCount {
  std::shared_ptr<T> m_ptr;

public:
  template <typename... TArgs>
  EnableRefCount(TArgs... args) : m_ptr(std::make_shared<T>(args...)) {}

  T *operator*() const { return m_ptr.operator*(); }
  T *operator->() const { return m_ptr.operator->(); }
  operator bool() const { return m_ptr.operator bool(); }

  // template<typename T2>
  friend struct std::hash<fun::EnableRefCount<T>>;
};

template <typename T>
class EnableWeakRefCount {
  std::weak_ptr<T> m_ptr;

public:
  EnableWeakRefCount(std::shared_ptr<T> ptr) : m_ptr(ptr) {}

  bool expired() const { return m_ptr.expired(); }

  EnableRefCount<T> lock() const { return m_ptr.lock(); }

  // template<typename T>
  friend struct std::hash<fun::EnableWeakRefCount<T>>;
};
}

namespace std {
template <typename T>
struct hash<fun::EnableRefCount<T>> {
private:
  hash<std::shared_ptr<T>> hasher;

public:
  std::size_t operator()(const fun::EnableRefCount<T> &ptr) const {
    return hasher(ptr.m_ptr);
  }
};

template <typename T>
struct hash<fun::EnableWeakRefCount<T>> {
private:
  hash<std::weak_ptr<T>> hasher;

public:
  std::size_t operator()(const fun::EnableWeakRefCount<T> &ptr) const {
    return hasher(ptr.m_ptr);
  }
};
}
