#include "refTracker.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "object.hpp"

#define RC_ACQUIRE(counter)                                                    \
  if (counter == COUNT_DESTROYING)                                             \
    throw std::runtime_error("use during free");                               \
  ++counter;                                                                   \
  if (counter == COUNT_DESTROYING)                                             \
    throw std::runtime_error("reference limit exceeded");

#define RC_REDUCE(counter, op)                                                 \
  if (counter == COUNT_DESTROYING) {                                           \
    std::cerr << "\e[1;38;5;3mwarning: \e[39m" << __func__                     \
              << "() called on FObject during free; ignoring\e[0m"             \
              << std::endl;                                                    \
    return;                                                                    \
  }                                                                            \
  if (!counter) throw std::runtime_error("double free");                       \
  op;                                                                          \
  assert(counter != COUNT_DESTROYING);                                         \
  tryDestroy();

#define RC_RELEASE(counter) RC_REDUCE(counter, --counter)
#define RC_RESET(counter) RC_REDUCE(counter, counter = 0)

namespace fun {
void FRefTracker::tryDestroy() {
  if (!m_tracked) {
    if (m_target) {
      m_tracked = COUNT_DESTROYING;
      delete m_target;
      m_tracked = 0;
      m_target  = nullptr;
    }

    if (!m_count) {
      m_count = COUNT_DESTROYING;
      delete this;
    }
  }
}

void FRefTracker::trackAcquire() { RC_ACQUIRE(m_tracked) }
void FRefTracker::trackRelease() { RC_RELEASE(m_tracked) }
void FRefTracker::trackReset() { RC_RESET(m_tracked) }

FRefTracker::FRefTracker(FObject *target) : m_target(target) {}

void FRefTracker::acquire() { RC_ACQUIRE(m_count) }
void FRefTracker::release() { RC_RELEASE(m_count) }
void FRefTracker::reset() { RC_RESET(m_count) }
}
