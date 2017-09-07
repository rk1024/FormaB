#pragma once

#include "intermedia/function.hpp"
#include "pipeline/stage.hpp"

namespace fie {
class FIOptimizer : public fps::FAccepts<FIFunction>,
                    public fps::FProduces<FIFunction> {

public:
  virtual void accept(fun::FPtr<const FIFunction>) override;
};
}
