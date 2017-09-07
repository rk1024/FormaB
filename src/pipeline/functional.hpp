#pragma once

#include "stage.hpp"

namespace fps {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wc++1z-extensions"
template <typename T,
          typename TReturn,
          bool isConst,
          template <typename> typename TContainer>
inline void connect(FProducesBase<TContainer, T, TReturn, isConst> &produces,
                    FAcceptsBase<TContainer, T, TReturn, isConst> & accepts) {
  produces.connect(fun::wrap(accepts));
}
#pragma clang diagnostic pop
}
