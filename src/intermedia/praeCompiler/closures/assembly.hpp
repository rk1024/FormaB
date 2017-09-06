#pragma once

#include "intermedia/function.hpp"

namespace fie {
namespace pc {
  class AssemblyClosure : public fun::FObject {
    fun::FAtomStore<fun::FPtr<FIFunction>, std::uint16_t> m_funcs;
    fun::FAtomStore<std::string, std::uint32_t>           m_msgs, m_strings;

  public:
    auto funcs() -> decltype(m_funcs) & { return m_funcs; }
    auto funcs() const -> const decltype(m_funcs) & { return m_funcs; }

    auto msgs() -> decltype(m_msgs) & { return m_msgs; }
    auto strings() -> decltype(m_strings) & { return m_strings; }
  };
}
}
