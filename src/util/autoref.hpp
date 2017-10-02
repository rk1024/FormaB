#pragma once

namespace fun {
template <typename>
class FPtr;

template <typename, typename, typename...>
class FMFPtr;

template <typename>
class FWeakPtr;

template <typename T>
struct _autoref {
  using type = const T &;
};

template <>
struct _autoref<void> {
  using type = void;
};

template <typename T>
struct _autoref<T *> {
  using type = T *;
};

template <typename T>
struct _autoref<T &> {
  using type = T &;
};

template <typename T>
struct _autoref<FPtr<T>> {
  using type = FPtr<T>;
};

template <typename T, typename U, typename... TArgs>
struct _autoref<FMFPtr<T, U, TArgs...>> {
  using type = FMFPtr<T, U, TArgs...>;
};

template <typename T>
struct _autoref<FWeakPtr<T>> {
  using type = FWeakPtr<T>;
};

template <typename T>
using autoref = typename _autoref<T>::type;
}
