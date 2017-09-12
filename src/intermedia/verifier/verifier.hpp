#pragma once

#include "util/ptr.hpp"

#include "pipeline/depends.hpp"

#include "intermedia/assembly.hpp"

namespace fie {
class FIVerifier : public fps::FDepends<fun::FPtr<FIFunction>> {
  fun::FPtr<FIAssembly> m_assem;

public:
  FIVerifier(fun::FPtr<FIAssembly> m_assem);

  virtual void accept(fun::FPtr<FIFunction>) override;
};
}
