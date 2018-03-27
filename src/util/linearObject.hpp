/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (linearObject.hpp)
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

namespace fun {
template <typename>
class FLinearPtr;

template <typename T>
class FLinearObject {
  FLinearPtr<T> *m_owner = nullptr;

  void own(FLinearPtr<T> *owner) {
    if (m_owner) m_owner->m_ptr = nullptr;
    m_owner = owner;
  }

  void disown(FLinearPtr<T> *
#ifndef NDEBUG
                  owner
#endif
  ) {
    assert(m_owner == owner);
    m_owner = nullptr;
  }

public:
  virtual ~FLinearObject() {}

  friend class FLinearPtr<T>;
};
} // namespace fun
