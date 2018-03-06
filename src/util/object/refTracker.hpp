/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (refTracker.hpp)
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

namespace fun {
class FObject;

class FRefTracker {
  unsigned int m_tracked = COUNT_UNCLAIMED, m_count = 0;
  FObject *    m_target = nullptr;

  void tryDestroy();

  void trackAcquire();
  void trackRelease();
  void trackReset();

public:
  static const unsigned int COUNT_DESTROYING = 0xffffffff,
                            COUNT_UNCLAIMED  = 0xfffffffe,
                            COUNT_MAX        = 0xfffffffd;

  inline FObject *    target() const { return m_target; }
  inline unsigned int trackedCount() const { return m_tracked; }
  inline unsigned int refCount() const { return m_count; }

  FRefTracker(FObject *target);

  FRefTracker(const FRefTracker &) = delete;
  FRefTracker(FRefTracker &)       = delete;
  FRefTracker(FRefTracker &&)      = delete;

  void acquire();
  void release();
  void reset();

  const FRefTracker &operator=(const FRefTracker &) = delete;
  const FRefTracker &operator=(FRefTracker &) = delete;
  const FRefTracker &operator=(FRefTracker &&) = delete;

  friend class FObject;
};
} // namespace fun
