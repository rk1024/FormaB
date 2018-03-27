/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (inputs.hpp)
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

#include <unordered_map>

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "ast/astBase.hpp"

#include "assembly.hpp"

namespace fie {
class FIInputs : public fun::FObject {
  fun::FPtr<FIAssembly>                                      m_assem;
  std::unordered_map<const frma::FormaAST *, FIFunctionAtom> m_funcs;
  std::unordered_map<const frma::FormaAST *, std::uint32_t>  m_structs;
  llvm::LLVMContext                                          m_llCtx;
  std::unique_ptr<llvm::Module>                              m_llModule;
  std::unordered_map<FIFunctionAtom, llvm::Function *>       m_llFuncs;
  std::unordered_map<FIStringAtom, llvm::GlobalVariable *>   m_llStrings;

public:
  constexpr auto &assem() { return m_assem; }
  constexpr auto &assem() const { return m_assem; }
  constexpr auto &funcs() { return m_funcs; }
  constexpr auto &funcs() const { return m_funcs; }
  constexpr auto &structs() { return m_structs; }
  constexpr auto &structs() const { return m_structs; }
  constexpr auto &llCtx() { return m_llCtx; }
  constexpr auto &llCtx() const { return m_llCtx; }
  constexpr auto &llModule() { return m_llModule; }
  constexpr auto &llModule() const { return m_llModule; }
  constexpr auto &llFuncs() { return m_llFuncs; }
  constexpr auto &llFuncs() const { return m_llFuncs; }
  constexpr auto &llStrings() { return m_llStrings; }
  constexpr auto &llStrings() const { return m_llStrings; }

  FIInputs(fun::FPtr<FIAssembly>, const std::string &, const std::string &);
};
} // namespace fie
