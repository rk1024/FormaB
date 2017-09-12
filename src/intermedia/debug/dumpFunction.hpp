#pragma once

#include <cstdint>
#include <iostream>

#include "pipeline/depends.hpp"

#include "intermedia/assembly.hpp"
#include "intermedia/function.hpp"
#include "intermedia/message.hpp"

namespace fie {
class FIDumpFunction : public fps::FDepends<fun::FPtr<FIFunction>> {
  fun::FPtr<FIAssembly> m_assem;
  std::ostream &        m_os;

public:
  FIDumpFunction(fun::FPtr<FIAssembly>, std::ostream &);

  virtual void accept(fun::FPtr<FIFunction>) override;
};
}
