/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (object.cpp)
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

#include "object.hpp"

#include <cassert>
#include <iostream>

namespace fun {
FObject::~FObject() {
  assert(tracker()->m_tracked);
  if (tracker()->m_tracked != FRefTracker::COUNT_DESTROYING) {
    std::cerr << "\e[1;38;5;3mwarning: \e[39mFObject not destroyed by its "
                 "tracker; detaching"
              << std::endl;

    tracker()->m_target = nullptr;
  }
}

void FObject::acquire() const { tracker()->trackAcquire(); }
void FObject::release() const { tracker()->trackRelease(); }
void FObject::reset() const { tracker()->trackReset(); }
}
