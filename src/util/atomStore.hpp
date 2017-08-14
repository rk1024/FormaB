#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

namespace fun {
template <typename T,
          typename TKey = std::size_t,
          bool useSet   = sizeof(std::size_t) < sizeof(TKey)>
class AtomStore;

template <typename T, typename TKey>
class AtomStore<T, TKey, false> {
  typename std::enable_if<(sizeof(std::size_t) >= sizeof(TKey) &&
                           std::is_integral<TKey>::value),
                          std::vector<T>>::type m_values;
  std::unordered_map<T, TKey> m_keys;
};

template <typename T, typename TKey>
class AtomStore<T, TKey, true> {
  typename std::enable_if<(sizeof(std::size_t) < sizeof(TKey) &&
                           std::is_integral<TKey>::value),
                          std::unordered_map<TKey, T>>::type m_values;
  std::unordered_map<T, TKey>                                m_keys;
};
}
