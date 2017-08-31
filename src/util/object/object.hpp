#pragma once

#include "refTracker.hpp"

namespace fun {
class FObject {
  FRefTracker *m_tracker = new FRefTracker(this);

public:
  inline FRefTracker *tracker() const { return m_tracker; }
  inline unsigned int refCount() const { return m_tracker->m_tracked; }

  virtual ~FObject();

  void acquire();
  void release();
};
}
