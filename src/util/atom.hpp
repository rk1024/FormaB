#pragma once

#include <cassert>
#include <functional>
#include <unordered_map>
#include <vector>

namespace fun {
template <typename T, typename TKey = std::size_t>
class FAtom {
public:
};

template <typename T,
          typename TKey = std::size_t,
          bool useSet   = sizeof(std::size_t) < sizeof(TKey)>
class FAtomStore;

template <typename T, typename TKey>
class FAtomStore<T, TKey, false> {
  typename std::enable_if<(sizeof(std::size_t) >= sizeof(TKey) &&
                           std::is_integral<TKey>::value),
                          std::vector<T>>::type m_values;
  std::unordered_map<T, TKey> m_keys;

  TKey add(T value) {
    TKey key = static_cast<TKey>(m_values.size());
    assert(key != static_cast<TKey>(-1));

    m_values.push_back(value);
    m_keys[value] = key;

    return key;
  }

public:
  TKey key(T value) const { return m_keys.at(value); }
  const T &value(TKey key) const { return m_values.at(key); }
  T &value(TKey key) { return m_values.at(key); }

  TKey size() const { return static_cast<TKey>(m_values.size()); }

  TKey intern(T value) {
    auto it = m_keys.find(value);
    if (it != m_keys.end()) return (*it).second;

    return add(value);
  }

  template <typename... TArgs>
  TKey emplace(TArgs... args) {
    return add(T(args...));
  }
};

template <typename T, typename TKey>
class FAtomStore<T, TKey, true> {
  typename std::enable_if<(sizeof(std::size_t) < sizeof(TKey) &&
                           std::is_integral<TKey>::value),
                          std::unordered_map<TKey, T>>::type m_values;

  TKey m_nextKey = 0;
  std::unordered_map<T, TKey> m_keys;

  TKey add(T value) {
    TKey key = m_nextKey;
    assert(key != static_cast<TKey>(-1));

    m_values[key] = value;
    m_keys[value] = key;
    ++m_nextKey;

    return key;
  }

public:
  TKey key(T value) const { return m_keys.at(value); }
  const T &value(TKey key) const { return m_values.at(key); }
  T &value(TKey key) { return m_values.at(key); }

  TKey size() const { return m_nextKey; }

  TKey intern(T value) {
    auto it = m_keys.find(value);
    if (it != m_keys.end()) return *it;

    return add(value);
  }

  template <typename... TArgs>
  TKey emplace(TArgs... args) {
    return add(T(args...));
  }
};
}
