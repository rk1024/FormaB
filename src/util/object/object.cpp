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

void FObject::acquire() { tracker()->trackAcquire(); }
void FObject::release() { tracker()->trackRelease(); }
void FObject::reset() { tracker()->trackReset(); }
}
