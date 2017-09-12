#pragma once

#include <cassert>
#include <functional>
#include <stdexcept>

#include "object/refTracker.hpp"
#include "util/hashing.hpp"

// #define FPTR_DIAGNOSTIC

#if defined(FPTR_DIAGNOSTIC)
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "util/atom.hpp"
#endif

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
#if defined(FPTR_DIAGNOSTIC)
extern FAtomStore<std::string> __fptr_t_ids;
extern std::unordered_map<std::size_t, FAtomStore<const void *>> __fptr_ids;
extern std::unordered_map<std::size_t, FAtomStore<const void *>> __fptr_o_ids;
extern unsigned int __fptr_indent_lvl;

void __fptr_clr_id(std::ostream &, std::size_t, bool);
void __fptr_log_id(std::ostream &, std::size_t, bool);

#define _FPTR_LOG(text)                                                        \
  do {                                                                         \
    auto __FPTR_T_ID = _FPTR_T_ID;                                             \
    auto flags       = std::cerr.flags();                                      \
    std::cerr << "\e[48;5;232m  [(FPtr) ";                                     \
    __fptr_clr_id(std::cerr, __FPTR_T_ID, false);                              \
    std::cerr << std::setw(32) << std::left << std::setfill(' ')               \
              << typeid(T).name() << "\e[39m #";                               \
    __fptr_log_id(std::cerr, __FPTR_ID, false);                                \
    std::cerr << " -> ";                                                       \
    if (this->m_ptr)                                                           \
      __fptr_log_id(std::cerr, __FPTR_O_ID, true);                             \
    else                                                                       \
      std::cerr << "\e[38;5;3m      n\e[39m";                                  \
    std::cerr.flags(flags);                                                    \
    std::cerr << " | ";                                                        \
    for (int i = 1; i < __fptr_indent_lvl; ++i) std::cerr << ": ";             \
    if (__fptr_indent_lvl) std::cerr << ":-";                                  \
    std::cerr << text << "] \e[m" << std::endl;                                \
  } while (false)

#define _FPTR_DIAG(expr) expr

#else

#define _FPTR_LOG(...)
#define _FPTR_DIAG(...)

#endif

#define _FPTR_T_ID _FPTR_DIAG(__fptr_t_ids.intern(typeid(T).name()))
#define __FPTR_T_ID _FPTR_DIAG(__t_id)

#define _FPTR_ID_T(t, obj) _FPTR_DIAG(__fptr_ids[_FPTR_T_ID].intern(obj))
#define _FPTR_ID_(obj) _FPTR_ID_T(_FPTR_T_ID, obj)
#define __FPTR_ID_(obj) _FPTR_ID_T(__FPTR_T_ID, obj)
#define _FPTR_ID _FPTR_ID_(this)
#define __FPTR_ID __FPTR_ID_(this)

#define _FPTR_O_ID_T(t, obj) _FPTR_DIAG(__fptr_o_ids[t].intern(obj->m_ptr))
#define _FPTR_O_ID_TD(t, obj) _FPTR_DIAG(__fptr_o_ids[t].intern(obj.m_ptr))
#define _FPTR_O_ID_(obj) _FPTR_O_ID_T(_FPTR_T_ID, obj)
#define _FPTR_O_ID_D(obj) _FPTR_O_ID_TD(_FPTR_T_ID, obj)
#define __FPTR_O_ID_(obj) _FPTR_O_ID_T(__FPTR_T_ID, obj)
#define __FPTR_O_ID_D(obj) _FPTR_O_ID_TD(__FPTR_T_ID, obj)
#define _FPTR_O_ID _FPTR_O_ID_(this)
#define __FPTR_O_ID __FPTR_O_ID_(this)

#define _FPTR_INDENT _FPTR_DIAG(++__fptr_indent_lvl)
#define _FPTR_OUTDENT _FPTR_DIAG(--__fptr_indent_lvl)

template <typename, typename, typename...>
class FMFPtr;

template <typename T>
class FPtr {
  T *m_ptr = nullptr;

#if defined(FPTR_DIAGNOSTIC)
  template <typename U>
  auto __fptr_outdent(U &&obj) {
    _FPTR_OUTDENT;
    return std::forward<U>(obj);
  }
#else
#define __fptr_outdent(x) x
#endif

public:
  inline T *get() const { return m_ptr; }

  explicit FPtr(T *ptr) : m_ptr(ptr) {
    if (ptr)
      _FPTR_LOG("T *(\e[38;5;5m" << __FPTR_O_ID << "\e[39m)");
    else
      _FPTR_LOG("T *(\e[38;5;3mn\e[39m)");
    _FPTR_INDENT;
    if (m_ptr) {
      _FPTR_DIAG(auto oldRefCount = m_ptr->refCount());
      m_ptr->acquire();
      _FPTR_LOG("\e[38;5;2macquire (" << oldRefCount << " :> "
                                      << m_ptr->refCount()
                                      << ")\e[39m");
    } else
      _FPTR_LOG("\e[38;5;3mnullptr\e[39m");
    _FPTR_OUTDENT;
  }

  FPtr(decltype(nullptr)) : FPtr() {} // Keep this implicit

  FPtr() : FPtr(static_cast<T *>(nullptr)) { _FPTR_LOG(">>n()"); }

  template <typename U>
  FPtr(const FPtr<U> &other) : FPtr(other.m_ptr) {}

  FPtr(const FPtr &other) : FPtr(other.m_ptr) {
    _FPTR_LOG(">>const copy(\e[38;5;6m" << __FPTR_ID_(&other)
                                        << "\e[39m -> \e[38;5;5m"
                                        << __FPTR_O_ID_D(other)
                                        << "\e[39m)");
  }

  FPtr(FPtr &other) : FPtr(const_cast<const FPtr &>(other)) {
    _FPTR_LOG(">>copy(\e[38;5;6m" << __FPTR_ID_(&other)
                                  << "\e[39m -> \e[38;5;5m"
                                  << __FPTR_O_ID_D(other)
                                  << "\e[39m)");
  }

  FPtr(FPtr &&other) : m_ptr(other.m_ptr) {
    _FPTR_LOG("move(\e[38;5;6m" << __FPTR_ID_(&other) << "\e[39m -> \e[38;5;5m"
                                << __FPTR_O_ID_D(other)
                                << "\e[39m)");
    other.m_ptr = nullptr;
  }

  ~FPtr() {
    _FPTR_LOG("dtor");
    _FPTR_INDENT;
    if (m_ptr) {
      _FPTR_LOG("\e[38;5;1mrelease (" << m_ptr->refCount() << " :> "
                                      << (m_ptr->refCount() - 1)
                                      << ")\e[39m");
      m_ptr->release();
    } else
      _FPTR_LOG("\e[38;5;3mnullptr\e[39m");
    m_ptr = nullptr;
    _FPTR_OUTDENT;
  }

  const FPtr &operator=(const FPtr &rhs) {
    _FPTR_LOG("const copy = \e[38;5;6m" << __FPTR_ID_(&rhs)
                                        << "\e[39m -> \e[38;5;5m"
                                        << __FPTR_O_ID_D(rhs)
                                        << "\e[39m");
    _FPTR_INDENT;
    if (&rhs == this) goto done;

    this->~FPtr();
    new (this) FPtr(rhs);
  done:
    _FPTR_OUTDENT;
    return *this;
  }

  const FPtr &operator=(FPtr &rhs) {
    _FPTR_LOG("copy = \e[38;5;6m" << __FPTR_ID_(&rhs) << "\e[39m -> \e[38;5;5m"
                                  << __FPTR_O_ID_D(rhs)
                                  << "\e[39m");
    _FPTR_INDENT;
    return __fptr_outdent(operator=(const_cast<const FPtr &>(rhs)));
  }

  const FPtr &operator=(FPtr &&rhs) {
    _FPTR_LOG("move = \e[38;5;6m" << __FPTR_ID_(&rhs) << "\e[39m -> \e[38;5;5m"
                                  << __FPTR_O_ID_D(rhs)
                                  << "\e[39m");
    _FPTR_INDENT;
    if (&rhs == this) goto done;

    this->~FPtr();
    new (this) FPtr(std::move(rhs));
  done:
    _FPTR_OUTDENT;
    return *this;
  }

  inline T *operator->() const {
    _FPTR_LOG("\e[38;5;4mop\e[39m ->");
    assert(m_ptr);
    return m_ptr;
  }
  inline T &operator*() const {
    _FPTR_LOG("\e[38;5;4mop\e[39m *");
    assert(m_ptr);
    return *m_ptr;
  }

  template <typename U>
  inline U operator->*(U T::*memb) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m ->*");
    assert(m_ptr);
    return m_ptr->*memb;
  }

  template <typename U, typename... TArgs>
  inline FMFPtr<T, U, TArgs...> operator->*(U (T::*memb)(TArgs...)) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m ->*");
    assert(m_ptr);
    return FMFPtr<T, U, TArgs...>(*this, memb);
  }

  inline bool operator==(const FPtr &rhs) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m == " << __FPTR_ID_(&rhs));
    return m_ptr == rhs.m_ptr;
  }
  inline bool operator!=(const FPtr &rhs) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m != " << __FPTR_ID_(&rhs));
    return m_ptr != rhs.m_ptr;
  }
  inline bool operator<(const FPtr &rhs) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m < " << __FPTR_ID_(&rhs));
    return m_ptr < rhs.m_ptr;
  }
  inline bool operator>(const FPtr &rhs) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m > " << __FPTR_ID_(&rhs));
    return m_ptr > rhs.m_ptr;
  }
  inline bool operator<=(const FPtr &rhs) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m <= " << __FPTR_ID_(&rhs));
    return m_ptr <= rhs.m_ptr;
  }
  inline bool operator>=(const FPtr &rhs) const {
    _FPTR_LOG("\e[38;5;4mop\e[39m >= " << __FPTR_ID_(&rhs));
    return m_ptr >= rhs.m_ptr;
  }

  inline bool operator!() const {
    _FPTR_LOG("\e[38;5;4mop\e[39m ! \e[38;5;6m" << (m_ptr ? "false" : "true")
                                                << "\e[39m");
    return !m_ptr;
  }
  inline operator bool() const {
    _FPTR_LOG("\e[38;5;4mbool\e[39m \e[38;5;6m" << (m_ptr ? "true" : "false")
                                                << "\e[39m");
    return !!m_ptr;
  }

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
    (m_ptr.m_ptr->*m_memb)(std::forward<TArgs>(args)...);
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

  FWeakPtr(const FWeakPtr<T> &) = default;
  FWeakPtr(FWeakPtr<T> &)       = default;
  FWeakPtr(FWeakPtr<T> &&)      = default;

  T *peek() const { return reinterpret_cast<T *>(m_ptr->target()); }

  FPtr<T> lock() const {
    if (!m_ptr) return FPtr<T>();

    switch (m_ptr->trackedCount()) {
    case FRefTracker::COUNT_DESTROYING:
      throw std::runtime_error("pointer use during free");
    case 0: throw std::runtime_error("pointer use after free");
    case FRefTracker::COUNT_UNCLAIMED:
    default: return wrap(reinterpret_cast<T *>(m_ptr->target()));
    }
  }

  FPtr<T> lockOrNull() const {
    if (!m_ptr) goto nil;

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

  bool operator!() const { return !operator bool(); }
  operator bool() const {
    if (!m_ptr) return false;

    switch (m_ptr->refCount()) {
    case FRefTracker::COUNT_DESTROYING:
    case 0: return false;
    case FRefTracker::COUNT_UNCLAIMED:
    default: return true;
    }
  }

  friend struct std::hash<FWeakPtr>;
};

template <typename T>
FWeakPtr<T> weak(const FPtr<T> &obj) {
  return FWeakPtr<T>(obj);
}

template <typename T>
FWeakPtr<T> weak(T *obj) {
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
  std::size_t operator()(const fun::FWeakPtr<T> &ptr) const {
    return fun::multiHash(ptr.m_ptr, fun::forward_hash(typeid(T).hash_code()));
  }
};
}

template <typename T, typename... TArgs>
fun::FPtr<T> fnew(TArgs &&... args) {
  return fun::FPtr<T>(new T(std::forward<TArgs>(args)...));
}
