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

  virtual ~FObject();

  void acquire();
  void release();
  void reset();

  friend class FRefTracker;
};
}
