/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (function.hpp)
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
#include <vector>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "block.hpp"
#include "intermedia/atoms.hpp"
#include "intermedia/messaging/message.hpp"
#include "intermedia/types/struct.hpp"
#include "variable.hpp"

namespace fie {
struct FIFunctionBody {
  std::vector<fun::FPtr<FIBlock>>            blocks;
  fun::FAtomStore<FIVariable, std::uint32_t> vars;
};

class FIFunction : public fun::FObject {
  FIMessage                                               m_msg;
  std::unordered_map<FIVariableAtom, fun::FPtr<FIStruct>> m_args;
  std::vector<FIVariableAtom>                             m_argOrder;
  FIFunctionBody                                          m_body;

public:
  constexpr const auto &msg() const { return m_msg; }
  constexpr const auto &args() const { return m_args; }
  constexpr const auto &argOrder() const { return m_argOrder; }
  constexpr const auto &body() const { return m_body; }

  FIFunction(
      const FIMessage &                                              msg,
      const std::unordered_map<FIVariableAtom, fun::FPtr<FIStruct>> &args,
      const std::vector<FIVariableAtom> &                            argOrder,
      const FIFunctionBody &                                         body) :
      m_msg(msg),
      m_args(args),
      m_argOrder(argOrder),
      m_body(body) {
    assert(m_args.size() == m_argOrder.size());
  }
};
} // namespace fie
