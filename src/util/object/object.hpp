/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (object.hpp)
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

#include "refTracker.hpp"

namespace fun {
class FObject {
  // NOTE: use tracker() instead of this internally
  FRefTracker *m_tracker = new FRefTracker(this);

public:
  inline FRefTracker *tracker() const {
    assert(m_tracker);
    return m_tracker;
  }
  inline unsigned int refCount() const {
    assert(m_tracker);
    return m_tracker->m_tracked;
  }

  FObject() = default;

  FObject(const FObject &) = default;
  FObject(FObject &)       = default;
  FObject(FObject &&)      = default;

  virtual ~FObject();

  void acquire() const;
  void release() const;
  void reset() const;

  friend class FRefTracker;
};
}
