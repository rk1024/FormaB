#pragma once

#include <cstdint>

#include "util/atom.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "function.hpp"
#include "messaging/message.hpp"

#include "types/interface.hpp"
#include "types/struct.hpp"

namespace fie {
class FIAssembly : public fun::FObject {
  fun::FAtomStore<fun::FPtr<const FIFunction>, std::uint32_t> m_funcs;
  fun::FAtomStore<std::string, std::uint32_t>                 m_strings;
  fun::FAtomStore<fun::FPtr<FIStruct>, std::uint32_t>         m_structs;
  fun::FAtomStore<fun::FPtr<FIInterface>, std::uint32_t>      m_interfaces;
  fun::FAtomStore<FIMessage, std::uint32_t>                   m_msgs;
  fun::FAtomStore<FIMessageKeyword, std::uint32_t>            m_keywords;

public:
  auto &      funcs() { return m_funcs; }
  const auto &funcs() const { return m_funcs; }
  auto &      strings() { return m_strings; }
  const auto &strings() const { return m_strings; }
  auto &      structs() { return m_structs; }
  const auto &structs() const { return m_structs; }
  auto &      interfaces() { return m_interfaces; }
  const auto &interfaces() const { return m_interfaces; }
  auto &      msgs() { return m_msgs; }
  const auto &msgs() const { return m_msgs; }
  auto &      keywords() { return m_keywords; }
  const auto &keywords() const { return m_keywords; }

  FIAssembly();
};
}
