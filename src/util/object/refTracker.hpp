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

  FRefTracker(const FObject &) = delete;
  FRefTracker(FObject &)       = delete;
  FRefTracker(FObject &&)      = delete;

  void acquire();
  void release();
  void reset();

  const FRefTracker &operator=(const FRefTracker &) = delete;
  const FRefTracker &operator=(FRefTracker &) = delete;
  const FRefTracker &operator=(FRefTracker &&) = delete;

  friend class FObject;
};
}
