/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (atom.hpp)
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
  std::unordered_map<T, TKey>                   m_keys;

public:
  inline TKey next() const { return static_cast<TKey>(m_values.size()); }

  inline TKey     key(T value) const { return m_keys.at(value); }
  inline const T &value(TKey key) const { return m_values.at(key); }
  inline T &      value(TKey key) { return m_values.at(key); }
  auto            begin() const { return m_keys.begin(); }
  auto            end() const { return m_keys.end(); }

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

  TKey                        m_nextKey = 0;
  std::unordered_map<T, TKey> m_keys;

public:
  inline TKey next() const { return m_nextKey; }

  inline TKey     key(T value) const { return m_keys.at(value); }
  inline const T &value(TKey key) const { return m_values.at(key); }
  inline T &      value(TKey key) { return m_values.at(key); }
  auto            begin() const { return m_keys.begin(); }
  auto            end() const { return m_keys.end(); }

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

template <typename T, typename TValue>
class FAtom {
  T m_value;

public:
  inline FAtom() : m_value() {}

  inline explicit FAtom(const T &value) : m_value(value) {}

  inline T &      value() { return m_value; }
  inline const T &value() const { return m_value; }

  bool operator==(const FAtom &rhs) const { return m_value == rhs.m_value; }
  bool operator!=(const FAtom &rhs) const { return m_value != rhs.m_value; }

  bool operator==(const T &rhs) const { return m_value == rhs; }
  bool operator!=(const T &rhs) const { return m_value != rhs; }

  friend bool operator==(const T &lhs, const FAtom &rhs) {
    return lhs == rhs.m_value;
  }
  friend bool operator!=(const T &lhs, const FAtom &rhs) {
    return lhs != rhs.m_value;
  }

  friend struct std::hash<FAtom>;
};

template <typename T, typename TKey = std::size_t>
class FAtomStore {
public:
  using Atom = FAtom<TKey, T>;

private:
  typedef FAtomStoreBuf<T, TKey> TBuf;

  TBuf m_buf;

  Atom add(const T &value) {
    TKey key = m_buf.next();
    assert(key != static_cast<TKey>(-1));

    m_buf.put(key, value);

    return Atom(key);
  }

public:
  Atom     key(const T &value) const { return Atom(m_buf.key(value)); }
  const T &value(Atom atom) const { return m_buf.value(atom.value()); }
  T &      value(Atom atom) { return m_buf.value(atom.value()); }
  auto     begin() const { return m_buf.begin(); }
  auto     end() const { return m_buf.end(); }

  TKey size() const { return m_buf.next(); }

  bool find(T value, Atom *atom) {
    TKey key;

    if (m_buf.find(value, &key)) {
      *atom = Atom(key);
      return true;
    }

    return false;
  }

  Atom intern(const T &value) {
    TKey key;
    if (m_buf.find(value, &key)) return Atom(key);

    return add(value);
  }

  bool intern(const T &value, Atom *atom) {
    TKey key;
    if (m_buf.find(value, key)) {
      *atom = key;
      return false;
    }

    *key  = add(value);
    *atom = key;
    return true;
  }

  template <typename... TArgs>
  Atom emplace(TArgs &&... args) {
    return add(T(std::forward<TArgs>(args)...));
  }
};
} // namespace fun

namespace std {
template <typename T, typename TValue>
struct hash<fun::FAtom<T, TValue>> {
  std::hash<T> m_hash;

public:
  inline auto operator()(const fun::FAtom<T, TValue> &atom) const {
    return m_hash(atom.m_value);
  }
};
} // namespace std
