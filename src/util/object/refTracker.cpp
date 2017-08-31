#include "refTracker.hpp"

#include <stdexcept>

#include "object.hpp"

namespace fun {
void FRefTracker::tryDestroy() {
  if (!m_tracked && !m_count) delete this;
}

void FRefTracker::trackAcquire() { ++m_tracked; }

void FRefTracker::trackRelease() {
  if (!m_tracked) throw std::runtime_error("double free");

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
