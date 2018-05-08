/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (scope.hpp)
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

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

namespace fie {
// TODO: This probably doesn't need to be refcounted
class FIScopeContainerBase : public fun::FObject {
public:
  virtual ~FIScopeContainerBase();

  virtual bool contains(const std::string &) const = 0;
};

template <typename T>
class FIScopeContainer : public FIScopeContainerBase {
  std::unordered_map<std::string, T> m_map;

public:
  constexpr auto &map() const { return m_map; }

  virtual bool contains(const std::string &name) const override {
    return m_map.find(name) != m_map.end();
  }

  template <typename... TArgs>
  void put(const std::string &name, TArgs &&... args) {
    m_map.emplace(name, T(std::forward<TArgs>(args)...));
  }
};

class FIScope : public fun::FObject {
  std::vector<fun::FPtr<FIScope>> m_parents;

  std::unordered_map<std::type_index, fun::FPtr<FIScopeContainerBase>>
                                                   m_containers;
  std::unordered_map<std::string, std::type_index> m_types;

public:
  bool contains(const std::string &name) const {
    return m_types.find(name) != m_types.end();
  }

  template <typename T, typename... TArgs>
  [[nodiscard]] bool put(const std::string &name, TArgs &&... args) {
    if (contains(name)) return false;

    fun::FPtr<FIScopeContainerBase> _container;

    if (auto it = m_containers.find(typeid(T)); it != m_containers.end())
      _container = it->second;
    else {
      _container = fnew<FIScopeContainer<T>>();
      m_containers.emplace(typeid(T), _container);
    }

    auto *container = m_containers.at(typeid(T)).as<FIScopeContainer<T>>();

    container->put(name, std::forward<TArgs>(args)...);

    m_types.emplace(name, typeid(T));

    return true;
  }

  template <typename T>
  FIScopeContainer<T> *container() const {
    return m_containers.at(typeid(T)).as<FIScopeContainer<T>>();
  }
};
} // namespace fie
