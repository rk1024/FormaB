/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (compiler.cpp)
* Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
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
*************************************************************************/

#include "compiler.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <stack>

#include "util/dumpHex.hpp"

#include "intermedia/messaging/builtins.hpp"
#include "intermedia/types/builtins.hpp"

using namespace frma;
using namespace fie::pc;

namespace fie {
#define _MOVE_2(id) __ctx_n##id##__
#define _MOVE_1(closure, to, id) auto _MOVE_2(id) = closure->move(to);

#define MOVE_TO_(closure, to) _MOVE_1(closure, to, __COUNTER__)
#define MOVE_(closure) MOVE_TO_(closure, node)
#define MOVE_TO(to) MOVE_TO_(closure, to)
#define MOVE MOVE_(closure)

#define SUBMOVE_(closure, sub) _MOVE_1(closure, node->sub, __COUNTER__)
#define SUBMOVE(sub) SUBMOVE_(closure, sub)

// Compiler emit header with FuncClosure, specify node type
#define EMITF_(name, type, ...)                                                \
  FIPraeCompiler::emit##name(                                                  \
      fun::FPtr<FuncClosure> closure, const FP##type *node, ##__VA_ARGS__)

// Compiler emit header with FuncClosure, matching node type
#define EMITF(name, ...) EMITF_(name, name, ##__VA_ARGS__)

// Compiler emitLoad header with FuncClosure, matching node type
#define EMITFL(name, ...) EMITF_(Load##name, name, ##__VA_ARGS__)

// Compiler emitStore header with FuncClosure, matching node type
#define EMITFS(name, ...) EMITF_(Store##name, name, ##__VA_ARGS__)

#define MATCH_(node) switch (node->alt())
#define MATCH MATCH_(node)

#define FAIL assert(false)
#define NOTIMPL closure->error("not implemented")
#define NEXT [[clang::fallthrough]]

template <typename T>
struct _fbasename;

template <typename T>
struct _fbasename<const T *> {
  typedef T type;
};

#define OF_(node, alt) _fbasename<decltype(node)>::type::alt
#define OF(alt) OF_(node, alt)
#define IS_(node, _alt) node->alt() == OF_(node, _alt)
#define IS(alt) IS_(node, alt)

std::uint32_t EMITF_(LoadExprsInternal, Exprs) {
  MOVE;
  std::uint32_t count = 0;

  MATCH {
  case OF(Empty): return 0;
  case OF(Exprs): count = emitLoadExprsInternal(closure, node->exprs()); NEXT;
  case OF(Expr):
    emitLoadExpr(closure, node->expr());
    ++count;
    break;
  default: FAIL;
  }

  return count;
}

void EMITFL(Exprs, bool tuple) {
  MOVE;
  std::uint32_t count = emitLoadExprsInternal(closure, node);
  if (tuple && count != 1) closure->emit(FIOpcode::Tpl, count);
}

void EMITFL(Expr) {
  MOVE;
  MATCH {
  case OF(Control): return emitLoadXControl(closure, node->ctl());
  case OF(Function): return emitLoadXFunc(closure, node->func());
  case OF(Infix): return emitLoadXInfix(closure, node->infix());
  default: FAIL;
  }
}

void EMITFL(LBoolean) {
  MOVE;
  MATCH {
  case OF(False): closure->emit<std::int32_t>(FIOpcode::Ldci4, 0); break;
  case OF(True): closure->emit<std::int32_t>(FIOpcode::Ldci4, 1); break;
  default: FAIL;
  }
}

void EMITFL(LNull) {
  MOVE;

  MATCH {
  case OF(Nil): closure->emit(FIOpcode::Ldnil); break;
  case OF(Void): closure->emit(FIOpcode::Ldvoid); break;
  }
}

void EMITFL(XBlock) {
  MOVE;
  MATCH {
  case OF(Block):
    closure->pushScope();

    emitStmts(closure, node->stmts());

    closure->applyScope();

    closure->emit(FIOpcode::Ldvoid);
    break;
  case OF(Error):
  default: FAIL;
  }
}

void EMITFL(XControl) {
  MOVE;
  bool invert = false;

  MATCH {
  case OF(If): break;
  case OF(Unless): invert = true; break;
  default: FAIL;
  }

  auto lblElse = closure->beginLabel(), lblDone = closure->beginLabel();

  closure->pushScope();

  emitLoadXParen(closure, node->cond(), ParenFlags::NoScope);

  closure->emit(
      FIInstruction::brLbl(invert ? FIOpcode::Bnz : FIOpcode::Bez, lblElse));

  emitLoadExpr(closure, node->then());

  auto phiVars = closure->applyScopeWithIds();

  closure->emit(FIInstruction::brLbl(FIOpcode::Br, lblDone));

  closure->label(lblElse);

  closure->pushScope();

  emitLoadExpr(closure, node->otherwise());

  closure->applyScopeWithIds(phiVars, false);

  closure->label(lblDone);
}

void EMITFL(XFunc) {
  MOVE;
  closure->emit(FIOpcode::Ldfun, m_inputs->funcs().at(node));
}

void EMITFL(XInfix) {
  MOVE;
  if (IS(Unary)) return emitLoadXUnary(closure, node->unary());

  emitLoadXInfix(closure, node->infixl());

  if (IS(Mod))
    emitLoadXUnary(closure, node->unary());
  else
    emitLoadXInfix(closure, node->infixr());

  MATCH {
  case OF(Add):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIAdd));
    break;
  case OF(Con):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FICon));
    break;
  case OF(Dis):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIDis));
    break;
  case OF(Div):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIDiv));
    break;
  case OF(Eql):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FICeq));
    break;
  case OF(Grt):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FICgt));
    break;
  case OF(Geq):
    closure->emit(FIOpcode::Msg, m_inputs->assem()->msgs().key(builtins::FIClt))
        .emit(FIOpcode::Msg, m_inputs->assem()->msgs().key(builtins::FIInv));
    break;
  case OF(Lss):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIClt));
    break;
  case OF(Leq):
    closure->emit(FIOpcode::Msg, m_inputs->assem()->msgs().key(builtins::FICgt))
        .emit(FIOpcode::Msg, m_inputs->assem()->msgs().key(builtins::FIInv));
    break;
  case OF(Mod):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIMod));
    break;
  case OF(Mul):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIMul));
    break;
  case OF(Neq):
    closure->emit(FIOpcode::Msg, m_inputs->assem()->msgs().key(builtins::FICeq))
        .emit(FIOpcode::Msg, m_inputs->assem()->msgs().key(builtins::FIInv));
    break;
  case OF(Sub):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FISub));
    break;
  case OF(Unary):
  default: FAIL;
  }
}

void EMITFL(XMember) {
  MOVE;
  MATCH {
  case OF(Member):
    NOTIMPL; // TODO
  case OF(Primary): return emitLoadXPrim(closure, node->prim());
  default: FAIL;
  }
}

void EMITFL(XMsg) {
  MOVE;

  emitLoadXPrim(closure, node->expr());

  MATCH {
  case OF(Coerce):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FICoerce));
    break;
  case OF(Curry): {
    auto sel = node->sel();

    MATCH_(sel) {
    case OF_(sel, Unary):
      closure
          ->emit(FIOpcode::Ldkw,
                 m_inputs->assem()->keywords().intern(
                     FIMessageKeyword(false, sel->tok()->toString())))
          .emit(FIOpcode::Msg,
                m_inputs->assem()->msgs().key(builtins::FICurry));
      break;
    case OF_(sel, Keyword): {
      std::stack<const FPMsgKeyword *> kwStack;
      const FPMsgKeywords *            kws = sel->kws();

      while (kws) {
        kwStack.push(kws->kw());
        kws = kws->kws();
      }

      while (kwStack.size()) {
        auto kw = kwStack.top();
        kwStack.pop();

        emitLoadExpr(closure, kw->expr());
        closure
            ->emit(FIOpcode::Ldkw,
                   m_inputs->assem()->keywords().intern(
                       FIMessageKeyword(true, kw->id()->toString())))
            .emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FICurry));
      }
      break;
    }
    }

    break;
  }
  case OF(Message): {
    auto               sel = node->sel();
    std::ostringstream oss;
    std::uint32_t      count = 1; // The receiver is the first parameter

    MATCH_(sel) {
    case OF_(sel, Unary): oss << sel->tok()->toString(); break;
    case OF_(sel, Keyword): {
      std::stack<const FPMsgKeyword *> kwStack;
      const FPMsgKeywords *            kws = sel->kws();

      while (kws) {
        kwStack.push(kws->kw());
        kws = kws->kws();
      }

      while (kwStack.size()) {
        auto kw = kwStack.top();
        kwStack.pop();
        oss << kw->id()->toString();
        ++count;
        emitLoadExpr(closure, kw->expr());
      }
      break;
    }
    default: FAIL;
    }

    closure->emit(
        FIOpcode::Msg,
        m_inputs->assem()->msgs().intern(FIMessage(count, oss.str())));

    break;
  }
  case OF(Error):
  default: FAIL;
  }
}

void EMITFL(XParen, ParenFlags::Flags flags) {
  MOVE;
  MATCH {
  case OF(Paren): emitLoadXParen(closure, node->paren(), flags); break;
  case OF(Tuple):
    if (flags & ParenFlags::Eval) emitLoadExprs(closure, node->exprs());
    break;
  case OF(Where): {
    if (flags & ParenFlags::Scope) closure->pushScope();

    if (flags & ParenFlags::Bind) emitSBind(closure, node->bind());
    if (flags & ParenFlags::Eval) emitLoadExpr(closure, node->expr());

    if (flags & ParenFlags::Scope) closure->dropScope();
    break;
  }
  case OF(Error):
  default: FAIL;
  }
}

void EMITFL(XPrim) {
  MOVE;
  MATCH {
  case OF(Block): return emitLoadXBlock(closure, node->block());
  case OF(Boolean): return emitLoadLBoolean(closure, node->boolean());
  case OF(Identifier):
    closure->emit(FIOpcode::Ldvar, closure->scope()->get(node->tok()->value()));
    break;
  case OF(Message): return emitLoadXMsg(closure, node->message());
  case OF(Null): return emitLoadLNull(closure, node->null());
  case OF(Numeric): return emitLoadLNumeric(closure, node->numeric());
  case OF(Parens): return emitLoadXParen(closure, node->paren());
  case OF(DQLiteral):
    closure->emit(FIOpcode::Ldstr,
                  m_inputs->assem()->strings().intern(node->tok()->toString()));
    break;
  case OF(SQLiteral): NOTIMPL;
  default: FAIL;
  }
}

void EMITFL(XUnary) {
  MOVE;
  const std::uint8_t I_Inc = 1, I_Dec = 2;

  if (IS(Member)) return emitLoadXMember(closure, node->memb());

  emitLoadXUnary(closure, node->unary());

  std::uint8_t inc = 0;

  MATCH {
  case OF(Dec): inc = I_Dec; break;
  case OF(Inc): inc = I_Inc; break;
  case OF(Inv):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIInv));
    break;
  case OF(Neg):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FINeg));
    break;
  case OF(Pos):
    closure->emit(FIOpcode::Msg,
                  m_inputs->assem()->msgs().key(builtins::FIPos));
    break;
  case OF(Member):
  default: FAIL;
  }

  if (inc) {
    closure->emit<std::int32_t>(FIOpcode::Ldci4, 1)
        .emit(FIOpcode::Msg,
              m_inputs->assem()->msgs().intern(inc == I_Inc ? builtins::FIAdd :
                                                              builtins::FISub))
        .emit(FIOpcode::Dup);

    emitStoreXUnary(closure, node->unary());
  }
}

void EMITFS(XMember) {
  MOVE;
  MATCH {
  case OF(Member):
    NOTIMPL; // TODO
  case OF(Primary): emitStoreXPrim(closure, node->prim()); break;
  default: FAIL;
  }
}

void EMITFS(XPrim) {
  MOVE;


  MATCH {
  case OF(Identifier):
    closure->emit(FIOpcode::Stvar, closure->scope()->set(node->tok()->value()));
    break;
  case OF(Block):
  case OF(Boolean):
  case OF(DQLiteral):
  case OF(Message):
  case OF(Null):
  case OF(Numeric):
  case OF(Parens):
  case OF(SQLiteral): closure->error("primary expression is not assignable");
  default: FAIL;
  }

  return;
}

void EMITFS(XUnary) {
  MOVE;

  MATCH {
  case OF(Dec):
  case OF(Inc):
  case OF(Inv):
  case OF(Neg):
  case OF(Pos): closure->error("expression is not assignable");
  case OF(Member): emitStoreXMember(closure, node->memb()); break;
  default: FAIL;
  }
}

void EMITF(Stmts) {
  MOVE;
  MATCH {
  case OF(Empty): break;
  case OF(Statements): emitStmts(closure, node->stmts()); NEXT;
  case OF(Statement): emitStmt(closure, node->stmt()); break;
  default: FAIL;
  }
}

void EMITF(Stmt) {
  MOVE;

  MATCH {
  case OF(Assign): emitLoadSAssign(closure, node->assign()); goto pop;
  case OF(Bind): emitSBind(closure, node->bind()); break;
  case OF(Control): emitSControl(closure, node->ctl()); break;
  case OF(Keyword): emitSKeyword(closure, node->kw()); break;
  case OF(NonSemiDecl):
  case OF(SemiDecl): emitDecl(closure, node->decl()); break;
  case OF(NonSemiExpr):
  case OF(SemiExpr): emitLoadExpr(closure, node->expr()); goto pop;
  case OF(Error):
  default: FAIL;
  }

  return;
pop:
  closure->emit(FIOpcode::Pop);
}

void EMITFL(SAssign) {
  MOVE;

  FIMessage op(0, "");

  MATCH {
  case OF(Add): op = builtins::FIAdd; break;
  case OF(Con): op = builtins::FICon; break;
  case OF(Dis): op = builtins::FIDis; break;
  case OF(Div): op = builtins::FIDiv; break;
  case OF(Mod): op = builtins::FIMod; break;
  case OF(Mul): op = builtins::FIMul; break;
  case OF(Sub): op = builtins::FISub; break;
  case OF(Assign): break;
  default: FAIL;
  }

  if (op.arity()) emitLoadXMember(closure, node->memb());

  emitLoadAssignValue(closure, node->value());

  if (op.arity())
    closure->emit(FIOpcode::Msg, m_inputs->assem()->msgs().intern(op));

  closure->emit(FIOpcode::Dup);

  emitStoreXMember(closure, node->memb());
}

void EMITF(SBind) {
  MOVE;
  MATCH {
  case OF(Let): emitBindings(closure, node->binds(), false); break;
  case OF(Var): emitBindings(closure, node->binds(), true); break;
  default: FAIL;
  }
}

void EMITF(SControl) {
  MOVE;
  const std::uint8_t C_Else = 0x1, C_Invert = 0x2, C_Loop = 0x4;

  std::uint8_t type = 0;

  MATCH {
  case OF(If): break;
  case OF(IfElse): type     = C_Else; break;
  case OF(Unless): type     = C_Invert; break;
  case OF(UnlessElse): type = C_Invert | C_Else; break;
  case OF(While): type      = C_Loop; break;
  case OF(WhileElse): type  = C_Loop | C_Else; break;
  case OF(Until): type      = C_Loop | C_Invert; break;
  case OF(UntilElse): type  = C_Loop | C_Invert | C_Else; break;
  default: FAIL;
  }

  if (type & C_Loop) {
    std::uint32_t lblDo    = closure->beginLabel(),
                  lblTest  = closure->beginLabel(),
                  lblBreak = closure->beginLabel();

    closure->pushScope();

    // TODO: Add proper phi-handling (i.e. id-recording) here
    //       NOTE: Currently uses standard mutable variables, but reassigns them
    emitLoadXParen(closure, node->cond(), ParenFlags::Bind);

    FuncClosure::VarIds loopPhiVars;

    for (auto var : closure->scope()->getOwned()) {
      loopPhiVars[fun::cons(fun::weak(closure->scope()), var.get<0>())] =
          closure->scope()->get(var.get<0>());
    }

    closure->emit(FIInstruction::brLbl(FIOpcode::Br, lblTest));

    closure->label(lblDo);

    closure->pushScope();

    emitStmt(closure, node->then());

    closure->applyScopeWithIds(loopPhiVars, false);

    closure->label(lblTest);

    closure->pushScope();

    emitLoadXParen(closure, node->cond(), ParenFlags::NoBind);

    closure->applyScopeWithIds(loopPhiVars, false);

    auto elsePhiVars = closure->applyScopeWithIds();

    closure->emit(FIInstruction::brLbl(
        type & C_Invert ? FIOpcode::Bez : FIOpcode::Bnz, lblDo));

    if (type & C_Else) {
      closure->pushScope();

      emitStmt(closure, node->otherwise());

      closure->applyScopeWithIds(elsePhiVars, false);
    }

    closure->label(lblBreak);
  } else {
    std::uint32_t lblElse = closure->beginLabel(), lblDone = -1;

    if (type & C_Else) lblDone = closure->beginLabel();

    closure->pushScope();

    emitLoadXParen(closure, node->cond(), ParenFlags::NoScope);

    closure->emit(FIInstruction::brLbl(
        type & C_Invert ? FIOpcode::Bnz : FIOpcode::Bez, lblElse));

    emitStmt(closure, node->then());

    auto phiVars = closure->applyScopeWithIds();

    if (type & C_Else)
      closure->emit(FIInstruction::brLbl(FIOpcode::Br, lblDone));

    closure->label(lblElse);

    if (type & C_Else) {
      closure->pushScope();

      emitStmt(closure, node->otherwise());

      closure->applyScopeWithIds(phiVars, false);
      closure->label(lblDone);
    }
  }
}

void EMITF(SKeyword) {
  MOVE;

  MATCH {
  case OF(Return): emitLoadExpr(closure, node->expr()); break;
  default: break;
  }

  MATCH {
  case OF(Return): closure->emit(FIOpcode::Ret); break;
  default: FAIL;
  }
}

void EMITFL(AssignValue) {
  MOVE;
  MATCH {
  case OF(Assign): return emitLoadSAssign(closure, node->assign());
  case OF(Infix): return emitLoadXInfix(closure, node->infix());
  default: FAIL;
  }
}

void EMITF(Bindings, bool mut) {
  MOVE;
  MATCH {
  case OF(Bindings): emitBindings(closure, node->binds(), mut); NEXT;
  case OF(Binding): emitBinding(closure, node->bind(), mut); break;
  default: FAIL;
  }
}

void EMITF(Binding, bool mut) {
  MOVE;
  emitLoadExpr(closure, node->expr());

  closure->emit<std::uint32_t>(
      FIOpcode::Stvar, closure->scope()->bind(node->id()->value(), mut));
}

void EMITF(Decl) {
  MOVE;
  MATCH {
  case OF(Message): emitDMsg(closure, node->msg()); break;
  case OF(Type): emitDType(closure, node->type()); break;
  default: FAIL;
  }
}

void EMITF(DMsg) {
  MOVE;

  auto               sel = node->sel();
  std::ostringstream oss;
  std::uint32_t      count = 1; // The receiver is the first parameter

  MATCH_(sel) {
  case OF_(sel, Unary): oss << sel->tok()->toString(); break;
  case OF_(sel, Keyword): {
    std::stack<const FPMsgDeclKeyword *> kwStack;
    const FPMsgDeclKeywords *            kws = sel->kws();

    while (kws) {
      MATCH_(kws) {
      case OF_(kws, Keywords):
        kwStack.push(kws->kw());
        kws = kws->kws();
        break;
      case OF_(kws, Keyword):
        kwStack.push(kws->kw());
        kws = nullptr;
        break;
      default: FAIL;
      }
    }

    while (kwStack.size()) {
      auto kw = kwStack.top();
      kwStack.pop();
      oss << kw->key()->toString();
      ++count;
      // TODO
    }
    break;
  }
  default: FAIL;
  }

  // TODO: Introduce this message to the current scope or something.
}

void EMITF(DType) {
  MOVE;

  MATCH {
  case OF(Interface): break;
  case OF(Struct): break;
  case OF(Variant): break;
  default: FAIL;
  }
}

FIPraeCompiler::FIPraeCompiler(fun::FPtr<FIInputs> inputs) : m_inputs(inputs) {}

std::uint32_t FIPraeCompiler::compileEntryPoint(
    fun::cons_cell<const FPStmts *> args) {
  FIBytecode body;
  auto       closure = fnew<FuncClosure>(body, args.get<0>());

  emitStmts(closure, args.get<0>());

  closure->emit(FIOpcode::Ldvoid).emit(FIOpcode::Ret);

  std::unordered_map<std::uint32_t, std::uint32_t> funcArgs;

  for (auto info : closure->args()->getOwned())
    funcArgs[closure->args()->get(info.get<0>())] = builtins::FIErrorT;

  auto id = m_inputs->assem()->funcs().intern(fnew<FIFunction>(funcArgs, body));
  m_inputs->funcs()[args.get<0>()] = id;

  return id;
}

std::uint32_t FIPraeCompiler::compileFunc(
    fun::cons_cell<const FPXFunc *> args) {
  FIBytecode body;
  auto       node    = args.get<0>();
  auto       closure = fnew<FuncClosure>(body, node);

  std::stack<const FPFuncParam *> paramStack;
  auto                            params = node->params();

  while (params) {
    MATCH_(params) {
    case OF_(params, List): params  = params->params(); break;
    case OF_(params, Empty): params = nullptr; break;
    case OF_(params, Parameters):
    case OF_(params, Parameter):
      paramStack.push(params->param());
      params = params->params();
      break;
    default: FAIL;
    }
  }

  while (paramStack.size()) {
    auto param = paramStack.top();
    auto pat   = param->pat();
    paramStack.pop();

    MATCH_(pat) {
    case OF_(pat, AnonAny):
    case OF_(pat, AnonPattern): break;
    case OF_(pat, NamedAny):
    case OF_(pat, NamedPattern):
      closure->args()->bind(pat->id()->value(), false);
      break;
    }
  }

  emitLoadExpr(closure, node->expr());

  closure->emit(FIOpcode::Ret);

  std::unordered_map<std::uint32_t, std::uint32_t> funcArgs;

  for (auto info : closure->args()->getOwned())
    funcArgs[closure->args()->get(info.get<0>())] = builtins::FIErrorT;

  auto id = m_inputs->assem()->funcs().intern(fnew<FIFunction>(funcArgs, body));
  m_inputs->funcs()[node] = id;

  return id;
}

std::uint32_t FIPraeCompiler::compileType(
    fun::cons_cell<const frma::FPDType *>) {
  return -1;
}
}
