/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (atom.hpp)
* Copyright (C) 2017 Ryan Schroeder, Colin Unger
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
*************************************************************************/

#pragma once

#include <cassert>
#include <functional>
#include <unordered_map>
#include <vector>

namespace fun {
template <typename T,
          typename TKey = std::size_t,
          bool useSet   = sizeof(std::size_t) < sizeof(TKey)>
class FAtomStoreBuf;

template <typename T, typename TKey>
class FAtomStoreBuf<T, TKey, false> {
  typename std::enable_if<(sizeof(std::size_t) >= sizeof(TKey) &&
                           std::is_integral<TKey>::value),
                          std::vector<T>>::type m_values;
  std::unordered_map<T, TKey> m_keys;

public:
  inline TKey next() const { return static_cast<TKey>(m_values.size()); }

  inline TKey key(T value) const { return m_keys.at(value); }
  inline const T &value(TKey key) const { return m_values.at(key); }
  inline T &value(TKey key) { return m_values.at(key); }

  inline bool find(T value, TKey *key) const {
    auto it = m_keys.find(value);
    if (it == m_keys.end()) return false;

    *key = it->second;
    return true;
  }

  inline void put(TKey key, T value) {
    m_values.push_back(value);
    m_keys[value] = key;
  }
};

template <typename T, typename TKey>
class FAtomStoreBuf<T, TKey, true> {
  typename std::enable_if<(sizeof(std::size_t) < sizeof(TKey) &&
                           std::is_integral<TKey>::value),
                          std::unordered_map<TKey, T>>::type m_values;

  TKey m_nextKey = 0;
  std::unordered_map<T, TKey> m_keys;

public:
  inline TKey next() const { return m_nextKey; }

  inline TKey key(T value) const { return m_keys.at(value); }
  inline const T &value(TKey key) const { return m_values.at(key); }
  inline T &value(TKey key) { return m_values.at(key); }

  inline bool find(T value, TKey *key) const {
    auto it = m_keys.find(value);
    if (it == m_keys.end()) return false;

    *key = it->second;
    return true;
  }

  inline void put(TKey key, T value) {
    m_values[key] = value;
    m_keys[value] = key;
    ++m_nextKey;
  }
};

template <typename T, typename TKey = std::size_t>
class FAtomStore {
  typedef FAtomStoreBuf<T, TKey> TBuf;

  TBuf m_buf;

  TKey add(const T &value) {
    TKey key = m_buf.next();
    assert(key != static_cast<TKey>(-1));

    m_buf.put(key, value);

    return key;
  }

public:
  TKey key(const T &value) const { return m_buf.key(value); }
  const T &value(TKey key) const { return m_buf.value(key); }
  T &value(TKey key) { return m_buf.value(key); }

  TKey size() const { return m_buf.next(); }

  bool find(T value, TKey *key) { return m_buf.find(value, key); }

  TKey intern(const T &value) {
    TKey key;
    if (m_buf.find(value, &key)) return key;

    return add(value);
  }

  bool intern(const T &value, TKey *key) {
    if (m_buf.find(value, key)) return false;

    *key = add(value);
    return true;
  }

  template <typename... TArgs>
  TKey emplace(TArgs &&... args) {
    return add(T(std::forward<TArgs>(args)...));
  }
};
}
