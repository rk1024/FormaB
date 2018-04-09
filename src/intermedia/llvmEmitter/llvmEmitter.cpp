/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (llvmEmitter.cpp)
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

#include "llvmEmitter.hpp"

#include <stack>

#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Verifier.h>

#include "util/compilerError.hpp"

#include "ast.hpp"
#include "ast/walker.hpp"

#include "intermedia/messaging/builtins.hpp"

namespace ll = llvm;

namespace fie {
void FILLVMEmitter::emitFunc(const std::string &name, FIFunctionAtom id) {
  auto  func = m_inputs->assem()->funcs().value(id);
  auto &body = func->body();

  auto &llCtx = m_inputs->llCtx();

  // TODO: Handle arguments
  auto llFuncType = ll::FunctionType::get(ll::Type::getDoubleTy(
                                              m_inputs->llCtx()),
                                          false);

  auto llFunc = ll::Function::Create(llFuncType,
                                     ll::Function::ExternalLinkage,
                                     name,
                                     m_inputs->llModule().get());

  m_inputs->llFuncs().emplace(id, llFunc);

  std::unordered_map<fun::FPtr<FIBlock>, ll::BasicBlock *> blocks;
  std::unordered_map<FIRegId, ll::Value *>                 regs;
  std::unordered_map<FIRegId, FIMessageKeywordAtom>        kws;
  std::unordered_map<FIVariableAtom, ll::Value *>          locals;

  for (int i = 0; i < body.blocks.size(); ++i) {
    auto &block   = body.blocks[i];
    auto  llBlock = ll::BasicBlock::Create(
        m_inputs->llCtx(), std::to_string(i) + "-" + block->name(), llFunc);

    blocks[block] = llBlock;

    m_llBuilder.SetInsertPoint(llBlock);

    for (auto &ins : block->body()) {
      auto &     val   = ins.value();
      ll::Value *llVal = nullptr;

      switch (val->opcode()) {
      case FIOpcode::Const: {
        switch (val.as<const FIConstantBase>()->constType()) {
        case FIConstType::Bool:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(8,
                        val.as<const FIConstant<bool>>()->value() ? 1 : 0,
                        false));
          break;

        case FIConstType::Float:
          llVal = ll::ConstantFP::get(
              llCtx, ll::APFloat(val.as<const FIConstant<float>>()->value()));
          break;
        case FIConstType::Double:
          llVal = ll::ConstantFP::get(
              llCtx, ll::APFloat(val.as<const FIConstant<double>>()->value()));
          break;

        case FIConstType::Int8:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(8,
                        val.as<const FIConstant<std::int8_t>>()->value(),
                        true));
          break;
        case FIConstType::Int16:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(16,
                        val.as<const FIConstant<std::int16_t>>()->value(),
                        true));
          break;
        case FIConstType::Int32:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(32,
                        val.as<const FIConstant<std::int32_t>>()->value(),
                        true));
          break;
        case FIConstType::Int64:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(64,
                        val.as<const FIConstant<std::int64_t>>()->value(),
                        true));
          break;

        case FIConstType::Uint8:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(8,
                        val.as<const FIConstant<std::uint8_t>>()->value(),
                        false));
          break;
        case FIConstType::Uint16:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(16,
                        val.as<const FIConstant<std::uint16_t>>()->value(),
                        false));
          break;
        case FIConstType::Uint32:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(32,
                        val.as<const FIConstant<std::uint32_t>>()->value(),
                        false));
          break;
        case FIConstType::Uint64:
          llVal = ll::ConstantInt::get(
              llCtx,
              ll::APInt(64,
                        val.as<const FIConstant<std::uint64_t>>()->value(),
                        false));
          break;

        case FIConstType::Func:
          llVal = m_inputs->llFuncs().at(
              val.as<const FIConstant<FIFunctionAtom>>()->value());
          break;
        case FIConstType::MsgKw:
          kws[ins.reg()] = val.as<const FIConstant<FIMessageKeywordAtom>>()
                               ->value();
          break;
        case FIConstType::String: {
          auto str = val.as<const FIConstant<FIStringAtom>>()->value();
          if (m_inputs->llStrings().find(str) == m_inputs->llStrings().end())
            m_inputs->llStrings().emplace(
                str,
                m_llBuilder.CreateGlobalString(
                    m_inputs->assem()->strings().value(str),
                    "s@" + std::to_string(str)));

          llVal = m_inputs->llStrings().at(str);

          break;
        }
        }
        break;
      }
      case FIOpcode::Ldvar:
        std::cerr << "WARNING: ldvar not supported\n";
        break;
      case FIOpcode::Msg: std::cerr << "WARNING: msg not supported\n"; break;
      case FIOpcode::Nil: std::cerr << "WARNING: nil not supported\n"; break;
      case FIOpcode::Phi: {
        auto phi = val.as<const FIPhiValue>();
        // TODO: Make this better, or do we even need to?
        auto llPhi = m_llBuilder.CreatePHI(
            regs[phi->values()[0].first]->getType(), phi->values().size());

        for (auto &[reg, blk] : phi->values())
          llPhi->addIncoming(regs[reg], blocks[blk.lock()]);
        std::cerr << "WARNING: phi not supported\n";
        break;
      }
      case FIOpcode::Stvar:
        std::cerr << "WARNING: stvar not supported\n";
        break;
      case FIOpcode::Tpl: std::cerr << "WARNING: tpl not supported\n"; break;
      case FIOpcode::Void: /* Leave this blank */ break;
      default: assert(false); break;
      }

      regs.emplace(ins.reg(), llVal);
    }
  }

  for (auto &block : body.blocks) {
    auto llBlock = blocks[block];
    m_llBuilder.SetInsertPoint(llBlock);

    switch (block->cont()) {
    case FIBlock::Branch:
      m_llBuilder.CreateCondBr(regs[block->ret()],
                               blocks[block->contA().lock()],
                               blocks[block->contB().lock()]);
      break;
    case FIBlock::Return: m_llBuilder.CreateRet(regs[block->ret()]); break;
    case FIBlock::Static:
      m_llBuilder.CreateBr(blocks[block->contA().lock()]);
      break;
    default: assert(false); break;
    }
  }

  if (ll::verifyFunction(*llFunc, &ll::errs())) std::cerr << std::endl;

  m_llBuilder.ClearInsertionPoint();
}

FILLVMEmitter::FILLVMEmitter(fun::FPtr<FIInputs> inputs) :
    m_inputs(inputs),
    m_llBuilder(m_inputs->llCtx()) {}

void FILLVMEmitter::emitFunc(fun::cons_cell<FIFunctionAtom> args) {
  emitFunc("stuff", args.get<0>());
}

void FILLVMEmitter::emitEntryPoint(fun::cons_cell<FIFunctionAtom> args) {
  emitFunc("o@main", args.get<0>());
}
} // namespace fie
