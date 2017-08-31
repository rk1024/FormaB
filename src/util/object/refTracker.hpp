#pragma once

namespace fun {
class FObject;

class FRefTracker {
  unsigned int m_tracked = 0, m_count = 0;
  FObject *    m_target = nullptr;

  void tryDestroy();
  void trackAcquire();
  void trackRelease();

public:
  inline bool     live() const { return !!m_tracked; }
  inline FObject *target() const { return m_target; }
  inline unsigned int refCount() const { return m_count; }

  FRefTracker(FObject *target);

  void acquire();
  void release();

  friend class FObject;
};
}
