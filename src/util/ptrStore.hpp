/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ptrStore.hpp)
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

#include <vector>

namespace fun {
template <typename T>
class FPtrStore {
  std::vector<T *> m_store;

public:
  constexpr auto &      store() { return m_store; }
  constexpr const auto &store() const { return m_store; }

  template <typename... TArgs>
  [[nodiscard]] decltype(auto) emplace(TArgs &&... args) {
    auto itm = new T(std::forward<TArgs>(args)...);
    m_store.push_back(itm);
    return itm;
  }

  template <typename U, typename... TArgs>
  [[nodiscard]] decltype(auto) emplaceP(TArgs &&... args) {
    auto itm = new U(std::forward<TArgs>(args)...);
    m_store.push_back(static_cast<T *>(itm));
    return itm;
  }

  ~FPtrStore() {
    for (auto ptr : m_store) delete ptr;
  }
};
} // namespace fun
