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
};

template <typename T>
class EnableWeakRefCount {
  std::weak_ptr<T> m_ptr;

public:
  EnableWeakRefCount(std::shared_ptr<T> ptr) : m_ptr(ptr) {}

  bool expired() const { return m_ptr.expired(); }

  EnableRefCount<T> lock() const { return m_ptr.lock(); }
};
}
