#include "refTracker.hpp"

#include "object.hpp"

namespace fun {
void FRefTracker::tryDestroy() {
  if (m_tracked == 0 && m_count == 0) delete this;
}

void FRefTracker::trackAcquire() { ++m_tracked; }

void FRefTracker::trackRelease() {
  --m_tracked;

  if (!m_tracked) {
    delete m_target;
    m_target = nullptr;
  }
}

FRefTracker::FRefTracker(FObject *target) : m_target(target) {}

void FRefTracker::acquire() { ++m_count; }

void FRefTracker::release() {
  --m_count;
  tryDestroy();
}
}
