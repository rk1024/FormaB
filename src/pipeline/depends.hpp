#pragma once

#include "util/autoref.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

namespace fps {
template <typename T, typename TRet = void>
class FDepends : public virtual fun::FObject {
public:
  virtual TRet accept(fun::autoref_t<T>) = 0;
};
}
