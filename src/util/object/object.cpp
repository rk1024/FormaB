/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (object.cpp)
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

#include "object.hpp"

#include <cassert>
#if !defined(NDEBUG)
#include <iostream>
#endif

namespace fun {
FObject::~FObject() {
#if !defined(NDEBUG)
  assert(tracker()->m_tracked);
  if (tracker()->m_tracked != FRefTracker::COUNT_DESTROYING) {
    std::cerr << "\x1b[1;38;5;3mwarning: \x1b[39mFObject not destroyed by its "
                 "tracker; detaching\x1b[m"
              << std::endl;

    tracker()->m_target = nullptr;
  }
#endif
}

void FObject::acquire() const { tracker()->trackAcquire(); }
void FObject::release() const { tracker()->trackRelease(); }
void FObject::reset() const { tracker()->trackReset(); }
} // namespace fun
