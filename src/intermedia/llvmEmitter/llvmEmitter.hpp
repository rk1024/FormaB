/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (llvmEmitter.hpp)
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

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include "util/cons.hpp"
#include "util/object/object.hpp"

#include "intermedia/inputs.hpp"


namespace fie {
class FILLVMEmitter : public fun::FObject {
  fun::FPtr<FIInputs>                                            m_inputs;
  llvm::IRBuilder<>                                              m_llBuilder;
  std::unordered_map<fun::FRef<const w::TypeBase>, llvm::Type *> m_llTypes;

  void emitFunc(const std::string &, FIFunctionAtom);

public:
  FILLVMEmitter(fun::FPtr<FIInputs>);

  void emitFunc(fun::cons_cell<FIFunctionAtom>);

  void emitEntryPoint(fun::cons_cell<FIFunctionAtom>);
};
} // namespace fie
