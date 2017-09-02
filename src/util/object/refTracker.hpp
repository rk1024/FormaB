#pragma once

namespace fun {
class FObject;

class FRefTracker {
  static const unsigned int COUNT_DESTROYING = 0xffffffff;

  static inline unsigned int getRefCount(unsigned int counter) {
    return counter == COUNT_DESTROYING ? 0 : counter;
  }

  unsigned int m_tracked = 0, m_count = 0;
  FObject *    m_target = nullptr;

  void tryDestroy();

  void trackAcquire();
  void trackRelease();
  void trackReset();

public:
  inline bool         live() const { return !!getRefCount(m_tracked); }
  inline FObject *    target() const { return m_target; }
  inline unsigned int refCount() const { return getRefCount(m_count); }

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
