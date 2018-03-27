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
  auto func = m_inputs->assem()->funcs().value(id);
  // auto &body = func->body();

  // TODO: Handle arguments
  auto llFuncType = ll::FunctionType::get(ll::Type::getDoubleTy(
                                              m_inputs->llCtx()),
                                          false);

  auto llFunc = ll::Function::Create(llFuncType,
                                     ll::Function::ExternalLinkage,
                                     name,
                                     m_inputs->llModule().get());

  m_inputs->llFuncs().emplace(id, llFunc);

  // auto llBB = ll::BasicBlock::Create(m_inputs->llCtx(), "_root", llFunc);
  // m_llBuilder.SetInsertPoint(llBB);

  // struct stack_el {
  //   enum { Void, Instruction, Value } type;

  //   union {
  //     ll::Value *      value;
  //     ll::Instruction *ins;
  //   };

  //   ll::Value *as_val() const {
  //     switch (type) {
  //     case Instruction: return ins;
  //     case Value: return value;
  //     default: return nullptr;
  //     }
  //   }

  //   stack_el() : type(Void) {}
  //   stack_el(ll::Value *v) : type(Value), value(v) {}
  //   stack_el(ll::Instruction *i) : type(Instruction), ins(i) {}

  //   stack_el(const stack_el &other) : type(other.type) {
  //     switch (other.type) {
  //     case Instruction: ins = other.ins; break;
  //     case Value: value = other.value; break;
  //     default: break;
  //     }
  //   }

  //   stack_el(stack_el &&other) : type(other.type) {
  //     switch (other.type) {
  //     case Instruction: ins = other.ins; break;
  //     case Value: value = other.value; break;
  //     default: break;
  //     }
  //   }

  //   operator ll::Value *() const { return as_val(); }
  //   operator bool() const { return type != Void; }
  // };

  std::unordered_map<FIRegId, ll::Value *>        regs;
  std::unordered_map<FIVariableAtom, ll::Value *> locals;

  std::cerr << "WARNING: LLVM emitter unimplemented\n";

  // for (auto &ins : body.instructions) {
  //   switch (ins->opcode()) {
  //   case FIOpcode::Nop: break;
  //   case FIOpcode::Dup: stack.emplace(stack.top()); break;
  //   case FIOpcode::Pop:
  //     if (stack.top().type == stack_el::Instruction)
  //       m_llBuilder.Insert(stack.top().ins);
  //     else if (stack.top()) {
  //       std::cerr << "\e[1mwarning:\e[0m voiding the following:\n";
  //       stack.top().as_val()->print(ll::errs(), true);
  //     }

  //     stack.pop();
  //     break;
  //   case FIOpcode::Ret:
  //     if (stack.top())
  //       m_llBuilder.CreateRet(stack.top());
  //     else
  //       m_llBuilder.CreateRetVoid();

  //     stack.pop();
  //     break;
  //   // case FIOpcode::Br: break;
  //   // case FIOpcode::Bez: break;
  //   // case FIOpcode::Bnz: break;
  //   case FIOpcode::Load: {
  //     auto type = structs.intern(ins.as<FILoadInstructionBase>()->type());

  //     if (type == tpInt8)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(8,
  //                     ins.as<FILoadInstruction<std::int8_t>>()->value(),
  //                     true)));
  //     else if (type == tpInt16)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(16,
  //                     ins.as<FILoadInstruction<std::int16_t>>()->value(),
  //                     true)));
  //     else if (type == tpInt32)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(32,
  //                     ins.as<FILoadInstruction<std::int32_t>>()->value(),
  //                     true)));
  //     else if (type == tpInt64)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(64,
  //                     ins.as<FILoadInstruction<std::int64_t>>()->value(),
  //                     true)));
  //     else if (type == tpUint8)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(8,
  //                     ins.as<FILoadInstruction<std::uint8_t>>()->value(),
  //                     false)));
  //     else if (type == tpUint16)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(16,
  //                     ins.as<FILoadInstruction<std::uint16_t>>()->value(),
  //                     false)));
  //     else if (type == tpUint32)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(32,
  //                     ins.as<FILoadInstruction<std::uint32_t>>()->value(),
  //                     false)));
  //     else if (type == tpUint64)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(64,
  //                     ins.as<FILoadInstruction<std::uint64_t>>()->value(),
  //                     false)));
  //     else if (type == tpFloat)
  //       stack.push(ll::ConstantFP::get(
  //           m_inputs->llCtx(),
  //           ll::APFloat(ins.as<FILoadInstruction<float>>()->value())));
  //     else if (type == tpDouble)
  //       stack.push(ll::ConstantFP::get(
  //           m_inputs->llCtx(),
  //           ll::APFloat(ins.as<FILoadInstruction<double>>()->value())));
  //     else if (type == tpBool)
  //       stack.push(ll::ConstantInt::get(
  //           m_inputs->llCtx(),
  //           ll::APInt(8, ins.as<FILoadInstruction<bool>>()->value(), false)));
  //     else if (type == tpString) {
  //       auto val = ins.as<FILoadInstruction<FIStringAtom>>()->value();
  //       if (m_inputs->llStrings().find(val) == m_inputs->llStrings().end())
  //         m_inputs->llStrings().emplace(
  //             val,
  //             m_llBuilder.CreateGlobalString(
  //                 m_inputs->assem()->strings().value(val),
  //                 "s@" + std::to_string(val.value())));

  //       stack.emplace(m_inputs->llStrings().at(val));
  //     }
  //     else
  //       abort();

  //     break;
  //   }
  //   case FIOpcode::Ldnil:
  //     stack.push(ll::ConstantPointerNull::get(
  //         ll::PointerType::get(ll::IntegerType::get(m_inputs->llCtx(), 32),
  //                              0)));
  //     break;
  //   case FIOpcode::Ldvoid: stack.emplace(); break;
  //   case FIOpcode::Ldvar:
  //     stack.emplace(locals.at(ins.as<FIVarInstruction>()->var()));
  //     break;
  //   case FIOpcode::Stvar:
  //     locals[ins.as<FIVarInstruction>()->var()] = std::move(
  //         stack.top()); // TODO: pretty sure this isn't right

  //     stack.pop();
  //     break;
  //   case FIOpcode::Msg: {
  //     auto msg = ins.as<FIMessageInstruction>()->msg();

  //     if (msg == msgAdd) {
  //       auto r = stack.top();
  //       stack.pop();
  //       auto l = stack.top();
  //       stack.pop();

  //       stack.push(m_llBuilder.CreateFAdd(l, r, "t@add"));
  //     }
  //     else if (msg == msgSub) {
  //       auto r = stack.top();
  //       stack.pop();
  //       auto l = stack.top();
  //       stack.pop();

  //       stack.push(m_llBuilder.CreateFSub(l, r, "t@sub"));
  //     }
  //     else if (msg == msgMul) {
  //       auto r = stack.top();
  //       stack.pop();
  //       auto l = stack.top();
  //       stack.pop();

  //       stack.push(m_llBuilder.CreateFMul(l, r, "t@mul"));
  //     }
  //     else if (msg == msgDiv) {
  //       auto r = stack.top();
  //       stack.pop();
  //       auto l = stack.top();
  //       stack.pop();

  //       stack.push(m_llBuilder.CreateFDiv(l, r, "t@div"));
  //     }
  //     else
  //       assert(false);

  //     break;
  //   }
  //   // case FIOpcode::Tpl: break;
  //   default: assert(false); break;
  //   }
  // }

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
