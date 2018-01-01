/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (assembly.hpp)
 * Copyright (C) 2017-2018 Ryan Schroeder, Colin Unger
 *
 * FormaB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * FormaB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
 *
 ************************************************************************/

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
} // namespace fie
