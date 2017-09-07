#pragma once

#include <cstdint>
#include <iostream>

#include "pipeline/stage.hpp"

#include "intermedia/function.hpp"
#include "intermedia/message.hpp"

namespace fie {
class FIDumpFunction : public fps::FAccepts<FIFunction, std::uint32_t>,
                       public fps::FAcceptsRef<std::string, std::uint32_t>,
                       public fps::FAcceptsRef<FIMessageId, std::uint32_t> {
  std::ostream &m_os;
  fun::FAtomStore<fun::FPtr<const FIFunction>, std::uint32_t> m_funcs;
  fun::FAtomStore<std::string, std::uint32_t>                 m_strings;
  fun::FAtomStore<FIMessageId, std::uint32_t>                 m_msgs;

public:
  FIDumpFunction(std::ostream &os);

  virtual std::uint32_t accept(fun::FPtr<const FIFunction>) override;
  virtual std::uint32_t accept(const std::string &) override;
  virtual std::uint32_t accept(const FIMessageId &) override;
};
}
