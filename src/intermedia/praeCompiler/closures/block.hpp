/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (block.hpp)
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

#include "util/linearObject.hpp"
#include "util/linearPtr.hpp"

#include "function.hpp"
#include "position.hpp"

namespace fie {
namespace pc {
  using RegResult  = std::pair<FIRegId, fun::FLinearPtr<BlockClosure>>;
  using VoidResult = fun::FLinearPtr<BlockClosure>;

  inline VoidResult voidReg(RegResult &&reg) { return reg.second.move(); }

  class BlockClosure : public fun::FLinearObject<BlockClosure> {
    fun::FPtr<FuncClosure> m_func;
    fun::FPtr<FIBlock>     m_block;

    auto regId() { return m_func->regId(); }

  public:
    constexpr auto &func() const { return m_func; }
    constexpr auto &scope() const { return m_func->scope(); }
    constexpr auto &block() const { return m_block; }

    auto curr() const { return m_func->curr(); }
    auto move(const frma::FormaAST *node) { return m_func->move(node); }
    [[noreturn]] void error(std::string &&desc) {
      m_func->error(std::move(desc));
    }

    VoidResult fork(const std::string &name) const {
      return flinear<BlockClosure>(m_func, name);
    }

    BlockClosure(const fun::FPtr<FuncClosure> &func, const std::string &name) :
        m_func(func),
        m_block(fnew<FIBlock>(name)) {
      m_func->body()->blocks.push_back(m_block);
    }

#ifndef NDEBUG
    virtual ~BlockClosure() override {
      if (m_block->cont() == FIBlock::ERR) {
        std::cerr << "ERROR: Block " << m_block->name() << " has cont() == ERR!"
                  << std::endl;
        abort();
      }
    }
#endif

    [[nodiscard]] RegResult emit(const FIInstruction &ins);
    [[nodiscard]] RegResult emit(FIInstruction &&ins);

    [[nodiscard]] decltype(auto) emitOp(const std::string &name, FIOpcode op) {
      return emit(
          FIInstruction(name, regId(), fnew<FIOpValue>(op, m_func->curr())));
    }

    template <typename T>
    [[nodiscard]] decltype(auto) emitConst(const std::string &name,
                                           const T &          value) {
      return emit(FIInstruction(name,
                                regId(),
                                fnew<FIConstant<T>>(value, m_func->curr())));
    }

    template <typename T>
    [[nodiscard]] decltype(auto) emitConst(const std::string &name, T &&value) {
      return emit(FIInstruction(name,
                                regId(),
                                fnew<FIConstant<T>>(value, m_func->curr())));
    }

        [[nodiscard]] decltype(auto) emitMsg(const std::string &         name,
                                             FIMessageAtom               msg,
                                             const std::vector<FIRegId> &args) {
      return emit(FIInstruction(name,
                                regId(),
                                fnew<FIMsgValue>(msg, args, m_func->curr())));
    }

    [[nodiscard]] decltype(auto) emitPhi(const std::string &         name,
                                         const std::vector<FIRegId> &values) {
      return emit(FIInstruction(name,
                                regId(),
                                fnew<FIPhiValue>(values, m_func->curr())));
    }

        [[nodiscard]] decltype(auto)
            emitTpl(const std::string &         name,
                    const std::vector<FIRegId> &values) {
      return emit(FIInstruction(name,
                                regId(),
                                fnew<FITplValue>(values, m_func->curr())));
    }

    [[nodiscard]] decltype(auto) emitLdvar(const std::string &name,
                                           FIVariableAtom     var) {
      return emit(FIInstruction(name,
                                regId(),
                                fnew<FILdvarValue>(var, m_func->curr())));
    }

        [[nodiscard]] decltype(auto) emitStvar(const std::string &name,
                                               FIVariableAtom     var,
                                               FIRegId            val) {
      return emit(FIInstruction(name,
                                regId(),
                                fnew<FIStvarValue>(var, val, m_func->curr())));
    }
  };
} // namespace pc
} // namespace fie
