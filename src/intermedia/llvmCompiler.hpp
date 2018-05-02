/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (llvmCompiler.hpp)
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

#include "util/object/object.hpp"

#include "const.hpp"
#include "context.hpp"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
// TODO: This is temporary
#include <llvm/Support/raw_ostream.h>

namespace fie {
class FILLVMCompiler : public fun::FObject {
  FIContext *       m_ctx;
  llvm::LLVMContext m_llCtx;
  // TODO: Should this even be a singleton?
  std::unique_ptr<llvm::Module> m_llModule;

public:
  FILLVMCompiler(FIContext &ctx, const std::string &moduleName) :
      m_ctx(&ctx),
      m_llModule(std::make_unique<llvm::Module>(moduleName, m_llCtx)) {}

  void compileGlobalConst(FIFoldedConst *);

  // TODO: This is also temporary
  void printModule() { m_llModule->print(llvm::errs(), nullptr); }
};
} // namespace fie
