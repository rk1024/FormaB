/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (refTracker.cpp)
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

#include "refTracker.hpp"

#include <cassert>
#include <stdexcept>
#if !defined(NDEBUG)
#include <iostream>
#endif

#include "object.hpp"

#define RC_ACQUIRE(counter)                                                    \
  switch (counter) {                                                           \
  case COUNT_DESTROYING: throw std::runtime_error("use during free");          \
  case COUNT_UNCLAIMED: counter = 1; break;                                    \
  default: ++counter; break;                                                   \
  }                                                                            \
                                                                               \
  if (counter > COUNT_MAX)                                                     \
    throw std::runtime_error(                                                  \
        "reference limit "                                                     \
        "exceeded");

#if defined(NDEBUG)
#define RC_WRNINFREE(fn)
#else
#define RC_WRNINFREE(fn)                                                       \
  std::cerr << "\e[1;38;5;3mwarning: \e[39m" << fn                             \
            << "() called on FObject during free; ignoring\e[m" << std::endl;
#endif

#define RC_REDUCE(counter, op)                                                 \
  switch (counter) {                                                           \
  case COUNT_DESTROYING: RC_WRNINFREE(__func__) return;                        \
  case COUNT_UNCLAIMED: throw std::runtime_error("free before claim");         \
  case 0: throw std::runtime_error("double free");                             \
  }                                                                            \
  op;                                                                          \
  assert(counter <= COUNT_MAX);                                                \
  tryDestroy();

#define RC_RELEASE(counter) RC_REDUCE(counter, --counter)
#define RC_RESET(counter) RC_REDUCE(counter, counter = 0)

namespace fun {
void FRefTracker::tryDestroy() {
  if (!m_tracked && m_target) {
    m_tracked = COUNT_DESTROYING;
    delete m_target;
    m_tracked = 0;
    m_target  = nullptr;
  }

  if (!(m_target || m_count)) {
    m_count = COUNT_DESTROYING;
    delete this;
  }
}

void FRefTracker::trackAcquire() { RC_ACQUIRE(m_tracked); }
void FRefTracker::trackRelease() { RC_RELEASE(m_tracked); }
void FRefTracker::trackReset() { RC_RESET(m_tracked); }

FRefTracker::FRefTracker(FObject *target) : m_target(target) {}

void FRefTracker::acquire() { RC_ACQUIRE(m_count); }
void FRefTracker::release() { RC_RELEASE(m_count); }
void FRefTracker::reset() { RC_RESET(m_count); }
} // namespace fun
