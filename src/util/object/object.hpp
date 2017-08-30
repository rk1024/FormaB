#pragma once

#include "refTracker.hpp"

namespace fun {
class FObject {
  FRefTracker *m_tracker = new FRefTracker(this);

public:
  inline FRefTracker *tracker() const { return m_tracker; }

  virtual ~FObject();

  void acquire();
  void release();
};
}
