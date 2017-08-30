#include "object.hpp"

namespace fun {
FObject::~FObject() {}

void FObject::acquire() { m_tracker->trackAcquire(); }

void FObject::release() { m_tracker->trackRelease(); }
}
