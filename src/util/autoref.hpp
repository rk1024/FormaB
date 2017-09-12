#pragma once

namespace fun {
template <typename>
class FPtr;

template <typename, typename, typename...>
class FMFPtr;

template <typename>
class FWeakPtr;

template <typename T>
struct autoref {
  using type = const T &;
};

template <typename T>
struct autoref<T *> {
  using type = T *;
};

template <typename T>
struct autoref<T &> {
  using type = T &;
};

template <typename T>
struct autoref<FPtr<T>> {
  using type = FPtr<T>;
};

template <typename T, typename U, typename... TArgs>
struct autoref<FMFPtr<T, U, TArgs...>> {
  using type = FMFPtr<T, U, TArgs...>;
};

template <typename T>
struct autoref<FWeakPtr<T>> {
  using type = FWeakPtr<T>;
};

template <typename T>
using autoref_t = typename autoref<T>::type;
}
