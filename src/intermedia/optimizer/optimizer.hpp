#pragma once

#include "pipeline/depends.hpp"

#include "intermedia/function.hpp"

namespace fie {
class FIOptimizer : public fps::FDepends<fun::FPtr<FIFunction>> {

public:
  virtual void accept(fun::FPtr<FIFunction>) override;
};
}
