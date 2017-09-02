#include "object.hpp"

#include <cassert>
#include <iostream>

namespace fun {
FObject::~FObject() {
  assert(m_tracker->m_tracked);
  if (m_tracker->m_tracked != FRefTracker::COUNT_DESTROYING) {
    std::cerr << "\e[1;38;5;3mwarning: \e[39mFObject not destroyed by its "
                 "tracker; detaching"
              << std::endl;

    m_tracker->m_target = nullptr;
  }
}

void FObject::acquire() { m_tracker->trackAcquire(); }
void FObject::release() { m_tracker->trackRelease(); }
void FObject::reset() { m_tracker->trackReset(); }
}
