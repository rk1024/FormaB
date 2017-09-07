#include "optimizer.hpp"

namespace fie {
void FIOptimizer::accept(fun::FPtr<const FIFunction> func) { propagate(func); }
}
