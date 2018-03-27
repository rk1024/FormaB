/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (compiler.cpp)
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

// Compiler emit header with BlockClosure, specify node type
#define EMITF_(name, type, ...)                                                \
  FIPraeCompiler::emit##name(fun::FLinearPtr<BlockClosure> closure,            \
                             const FP##type *              node,               \
                             ##__VA_ARGS__)

// Compiler emit header with BlockClosure, matching node type
#define EMITF(name, ...) EMITF_(name, name, ##__VA_ARGS__)

// Compiler emitLoad header with BlockClosure, matching node type
#define EMITFL(name, ...) RegResult EMITF_(Load##name, name, ##__VA_ARGS__)

// Compiler emit header with void return and BlockClosure, specify node type
#define EMITFV_(name, type, ...) VoidResult EMITF_(name, type, ##__VA_ARGS__)

// Compiler emit header with void return and BlockClosure, matching node type
#define EMITFV(name, ...) EMITFV_(name, name, ##__VA_ARGS__)

// Compiler emitStore header with BlockClosure, matching node type
#define EMITFS(name, ...) EMITFV_(Store##name, name, ##__VA_ARGS__)

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

EMITFV_(LoadExprsInternal, Exprs, std::vector<FIRegId> &regs) {
  MOVE;
  MATCH {
  case OF(Empty): return closure;
  case OF(Exprs):
    closure = emitLoadExprsInternal(closure.move(), node->exprs(), regs);
    NEXT;
  case OF(Expr): {
    auto [reg, closure2] = emitLoadExpr(closure.move(), node->expr());
    regs.push_back(reg);
    return closure2.move();
  }
  default: FAIL;
  }
}

EMITFL(Exprs, bool tuple) {
  MOVE;
  std::vector<FIRegId> regs;
  auto closure2 = emitLoadExprsInternal(closure.move(), node, regs);
  if (tuple && regs.size() != 1) return closure->emitTpl("tpl", regs);
  assert(regs.size() == 1);
  return RegResult(regs.front(), closure2.move());
}

EMITFL(Expr) {
  MOVE;
  MATCH {
  case OF(Control): return emitLoadXControl(closure.move(), node->ctl());
  case OF(Function): return emitLoadXFunc(closure.move(), node->func());
  case OF(Infix): return emitLoadXInfix(closure.move(), node->infix());
  default: FAIL;
  }
}

EMITFL(LBoolean) {
  MOVE;
  MATCH {
  case OF(False): return closure->emitConst<bool>("false", false);
  case OF(True): return closure->emitConst<bool>("true", true);
  default: FAIL;
  }
}

EMITFL(LNull) {
  MOVE;

  MATCH {
  case OF(Nil): return closure->emitOp("nil", FIOpcode::Nil);
  case OF(Void): return closure->emitOp("void", FIOpcode::Void);
  default: FAIL;
  }
}

EMITFL(XBlock) {
  MOVE;
  MATCH {
  case OF(Block): {
    closure->func()->pushScope();

    auto closure2 = emitStmts(closure.move(), node->stmts());

    closure2->func()->dropScope();

    return closure2->emitOp("block", FIOpcode::Void);
  }
  case OF(Error): abort();
  default: FAIL;
  }
}

EMITFL(XControl) {
  MOVE;
  bool invert = false;

  MATCH {
  case OF(If): break;
  case OF(Unless): invert = true; break;
  default: FAIL;
  }

  // NB: This stuff is order-dependent.
  auto blkThen      = closure->fork("xi-then");
  auto blkOtherwise = closure->fork("xi-else");
  auto blkDone      = closure->fork("xi-end");

  closure->func()->pushScope();

  auto [cond, closure2] = emitLoadXParen(closure.move(), node->cond(), false);

  closure2->block()->contBranch(cond,
                                invert,
                                blkThen->block(),
                                blkOtherwise->block());
  blkThen->block()->contStatic(blkDone->block());
  blkOtherwise->block()->contStatic(blkDone->block());

  auto [then, blkThen2] = emitLoadExpr(blkThen.move(), node->then());

  closure2->func()->dropScope();

  closure2->func()->pushScope();

  auto [otherwise, blkOtherwise2] = emitLoadExpr(blkOtherwise.move(),
                                                 node->otherwise());

  closure2->func()->dropScope();

  return blkDone->emitPhi("xi-phi", {then, otherwise});
}

EMITFL(XFunc) {
  MOVE;
  return closure->emitConst<FIFunctionAtom>("func", m_inputs->funcs().at(node));
}

EMITFL(XInfix) {
  MOVE;
  if (IS(Unary)) return emitLoadXUnary(closure.move(), node->unary());

  auto [lhs, closure2] = emitLoadXInfix(closure.move(), node->infixl());

  std::vector<FIRegId> args{lhs};

  RegResult rhsResult;

  if (IS(Mod))
    rhsResult = emitLoadXUnary(closure2.move(), node->unary());
  else
    rhsResult = emitLoadXInfix(closure2.move(), node->infixr());

  auto &[rhs, closure3] = rhsResult;

  args.push_back(rhs);

  MATCH {
  case OF(Add):
    return closure3->emitMsg("add",
                             m_inputs->assem()->msgs().key(builtins::FIAdd),
                             args);
  case OF(Con):
    return closure3->emitMsg("sub",
                             m_inputs->assem()->msgs().key(builtins::FICon),
                             args);
  case OF(Dis):
    return closure3->emitMsg("dis",
                             m_inputs->assem()->msgs().key(builtins::FIDis),
                             args);
  case OF(Div):
    return closure3->emitMsg("div",
                             m_inputs->assem()->msgs().key(builtins::FIDiv),
                             args);
  case OF(Eql):
    return closure3->emitMsg("eql",
                             m_inputs->assem()->msgs().key(builtins::FICeq),
                             args);
  case OF(Grt):
    return closure3->emitMsg("grt",
                             m_inputs->assem()->msgs().key(builtins::FICgt),
                             args);
  case OF(Geq): {
    auto [val, closure4] = closure3->emitMsg(
        "geq", m_inputs->assem()->msgs().key(builtins::FIClt), args);
    return closure4->emitMsg("geq",
                             m_inputs->assem()->msgs().key(builtins::FIInv),
                             {val});
  }
  case OF(Lss):
    return closure3->emitMsg("lss",
                             m_inputs->assem()->msgs().key(builtins::FIClt),
                             args);
  case OF(Leq): {
    auto [val, closure4] = closure3->emitMsg(
        "leq", m_inputs->assem()->msgs().key(builtins::FICgt), args);
    return closure4->emitMsg("leq",
                             m_inputs->assem()->msgs().key(builtins::FIInv),
                             {val});
  }
  case OF(Mod):
    return closure3->emitMsg("mod",
                             m_inputs->assem()->msgs().key(builtins::FIMod),
                             args);
  case OF(Mul):
    return closure3->emitMsg("mul",
                             m_inputs->assem()->msgs().key(builtins::FIMul),
                             args);
  case OF(Neq): {
    auto [val, closure4] = closure3->emitMsg(
        "neq", m_inputs->assem()->msgs().key(builtins::FICeq), args);
    return closure4->emitMsg("neq",
                             m_inputs->assem()->msgs().key(builtins::FIInv),
                             {val});
  }
  case OF(Sub):
    return closure3->emitMsg("sub",
                             m_inputs->assem()->msgs().key(builtins::FISub),
                             args);
  case OF(Unary): abort();
  default: FAIL;
  }
}

EMITFL(XMember) {
  MOVE;
  MATCH {
  case OF(Member): NOTIMPL; // TODO
  case OF(Primary): return emitLoadXPrim(closure.move(), node->prim());
  default: FAIL;
  }
}

EMITFL(XMsg) {
  MOVE;

  auto [recv, closure2] = emitLoadXPrim(closure.move(), node->expr());

  std::vector<FIRegId> args{recv};

  MATCH {
  case OF(Coerce):
    return closure2->emitMsg("coerce",
                             m_inputs->assem()->msgs().key(builtins::FICoerce),
                             args);
  case OF(Curry): {
    auto sel = node->sel();

    MATCH_(sel) {
    case OF_(sel, Unary): {
      auto [kw, closure3] = closure2->emitConst<FIMessageKeywordAtom>(
          "curry",
          m_inputs->assem()->keywords().intern(
              FIMessageKeyword(false, sel->tok()->toString())));
      args.push_back(kw);
      return closure3->emitMsg("curry",
                               m_inputs->assem()->msgs().key(builtins::FICurry),
                               args);
    }
    case OF_(sel, Keyword): {
      std::stack<const FPMsgKeyword *> kwStack;
      const FPMsgKeywords *            kws = sel->kws();

      while (kws) {
        kwStack.push(kws->kw());
        kws = kws->kws();
      }

      assert(args.size() == 1);
      FIRegId ret = args.front();

      while (kwStack.size()) {
        auto kw = kwStack.top();
        kwStack.pop();

        auto [recv, closure3]  = emitLoadExpr(closure2.move(), kw->expr());
        auto [kwReg, closure4] = closure3->emitConst<FIMessageKeywordAtom>(
            "currykw",
            m_inputs->assem()->keywords().intern(
                FIMessageKeyword(true, kw->id()->toString())));

        args = {ret, recv, kwReg};

        auto [retReg, closure5] = closure4->emitMsg(
            "curry", m_inputs->assem()->msgs().key(builtins::FICurry), args);

        ret = retReg;

        closure2 = closure5.move();
      }

      return RegResult(ret, closure2.move());
    }
    }

    break;
  }
  case OF(Message): {
    auto               sel = node->sel();
    std::ostringstream oss;

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
        auto [arg, closure3] = emitLoadExpr(closure2.move(), kw->expr());
        closure2             = closure3.move();
        args.push_back(arg);
      }
      break;
    }
    default: FAIL;
    }

    return closure2->emitMsg("msg",
                             m_inputs->assem()->msgs().intern(
                                 FIMessage(args.size(), oss.str())),
                             args);
  }
  case OF(Error): abort();
  default: FAIL;
  }
}

EMITFL(XParen, bool scope) {
  MOVE;
  MATCH {
  case OF(Paren): return emitLoadXParen(closure.move(), node->paren(), scope);
  case OF(Tuple): return emitLoadExprs(closure.move(), node->exprs());
  case OF(Where): {
    if (scope) closure->func()->pushScope();

    auto closure2 = emitSBind(closure.move(), node->bind());
    auto ret      = emitLoadExpr(closure2.move(), node->expr());

    auto &[_, closure3] = ret;

    if (scope) closure3->func()->dropScope();
    return ret;
  }
  case OF(Error): abort();
  default: FAIL;
  }
}

EMITFL(XPrim) {
  MOVE;
  MATCH {
  case OF(Block): return emitLoadXBlock(closure.move(), node->block());
  case OF(Boolean): return emitLoadLBoolean(closure.move(), node->boolean());
  case OF(Identifier):
    return closure->emitLdvar("ldvar",
                              closure->scope()->get(node->tok()->value()));
  case OF(Message): return emitLoadXMsg(closure.move(), node->message());
  case OF(Null): return emitLoadLNull(closure.move(), node->null());
  case OF(Numeric): return emitLoadLNumeric(closure.move(), node->numeric());
  case OF(Parens): return emitLoadXParen(closure.move(), node->paren());
  case OF(DQLiteral):
    return closure->emitConst<FIStringAtom>("dqlit",
                                            m_inputs->assem()->strings().intern(
                                                node->tok()->toString()));
  case OF(SQLiteral): NOTIMPL;
  default: FAIL;
  }
}

EMITFL(XUnary) {
  MOVE;
  const std::uint8_t I_Inc = 1, I_Dec = 2;

  if (IS(Member)) return emitLoadXMember(closure.move(), node->memb());

  auto [recv, closure2] = emitLoadXUnary(closure.move(), node->unary());

  std::vector<FIRegId> args{recv};

  std::uint8_t inc = 0;

  MATCH {
  case OF(Dec): inc = I_Dec; break;
  case OF(Inc): inc = I_Inc; break;
  case OF(Inv):
    return closure->emitMsg("inv",
                            m_inputs->assem()->msgs().key(builtins::FIInv),
                            args);
  case OF(Neg):
    return closure->emitMsg("neg",
                            m_inputs->assem()->msgs().key(builtins::FINeg),
                            args);
  case OF(Pos):
    return closure->emitMsg("pos",
                            m_inputs->assem()->msgs().key(builtins::FIPos),
                            args);
  case OF(Member):
  default: FAIL;
  }

  if (inc) {
    auto [arg, closure3] = closure2->emitConst<std::int32_t>("inc", 1);
    args.push_back(arg);
    auto [ret,
          closure4] = closure3->emitMsg("inc",
                                        m_inputs->assem()->msgs().intern(
                                            inc == I_Inc ? builtins::FIAdd
                                                         : builtins::FISub),
                                        args);

    auto closure5 = emitStoreXUnary(closure4.move(), node->unary(), ret);
    return RegResult(ret, closure5.move());
  }

  abort(); // You really shouldn't be here.
}

EMITFS(XMember, const FIRegId &val) {
  MOVE;
  MATCH {
  case OF(Member): NOTIMPL; // TODO
  case OF(Primary): return emitStoreXPrim(closure.move(), node->prim(), val);
  default: FAIL;
  }
}

EMITFS(XPrim, const FIRegId &val) {
  MOVE;


  MATCH {
  case OF(Identifier):
    return voidReg(closure->emitStvar(
        "stvar", closure->scope()->set(node->tok()->value()), val));
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
}

EMITFS(XUnary, const FIRegId &val) {
  MOVE;

  MATCH {
  case OF(Dec):
  case OF(Inc):
  case OF(Inv):
  case OF(Neg):
  case OF(Pos): closure->error("expression is not assignable");
  case OF(Member): return emitStoreXMember(closure.move(), node->memb(), val);
  default: FAIL;
  }
}

EMITFV(Stmts) {
  MOVE;
  MATCH {
  case OF(Empty): return closure;
  case OF(Statements): closure = emitStmts(closure.move(), node->stmts()); NEXT;
  case OF(Statement): return emitStmt(closure.move(), node->stmt());
  default: FAIL;
  }
}

EMITFV(Stmt) {
  MOVE;

  MATCH {
  case OF(Assign):
    return voidReg(emitLoadSAssign(closure.move(), node->assign()));
  case OF(Bind): return emitSBind(closure.move(), node->bind());
  case OF(Control): return emitSControl(closure.move(), node->ctl());
  case OF(Keyword): return emitSKeyword(closure.move(), node->kw());
  case OF(NonSemiDecl):
  case OF(SemiDecl): return emitDecl(closure.move(), node->decl());
  case OF(NonSemiExpr):
  case OF(SemiExpr): return voidReg(emitLoadExpr(closure.move(), node->expr()));
  case OF(Error): abort();
  default: FAIL;
  }
}

EMITFL(SAssign) {
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

  RegResult valResult;

  if (op.arity()) {
    auto [a, closure2] = emitLoadXMember(closure.move(), node->memb());
    auto [b, closure3] = emitLoadAssignValue(closure2.move(), node->value());

    valResult = closure3->emitMsg("asgop",
                                  m_inputs->assem()->msgs().intern(op),
                                  {a, b});
  }
  else
    valResult = emitLoadAssignValue(closure.move(), node->value());

  auto &[val, closure2] = valResult;

  auto closure3 = emitStoreXMember(closure2.move(), node->memb(), val);

  return RegResult(val, closure3.move());
}

EMITFV(SBind) {
  MOVE;
  MATCH {
  case OF(Let): return emitBindings(closure.move(), node->binds(), false);
  case OF(Var): return emitBindings(closure.move(), node->binds(), true);
  default: FAIL;
  }
}

EMITFV(SControl) {
  MOVE;
  const std::uint8_t C_Else = 0x1, C_Invert = 0x2, C_Loop = 0x4;

  std::uint8_t type = 0;

  MATCH {
  case OF(If): break;
  case OF(IfElse): type = C_Else; break;
  case OF(Unless): type = C_Invert; break;
  case OF(UnlessElse): type = C_Invert | C_Else; break;
  case OF(While): type = C_Loop; break;
  case OF(WhileElse): type = C_Loop | C_Else; break;
  case OF(Until): type = C_Loop | C_Invert; break;
  case OF(UntilElse): type = C_Loop | C_Invert | C_Else; break;
  default: FAIL;
  }

  if (type & C_Loop) {
    // NB: This stuff is order-dependent
    auto  blkThen      = closure->fork("sw-then");
    auto  blkTest      = closure->fork("sw-test");
    auto  blkOtherwise = type & C_Else ? closure->fork("sw-else") : nullptr;
    auto  blkEnd       = closure->fork("sw-end");
    auto &blkDone      = type & C_Else ? blkOtherwise : blkEnd;

    closure->func()->pushScope();

    std::cerr << closure->block()->name() << std::endl;

    auto _blkTest = blkTest->block();

    closure->block()->contStatic(blkTest->block());

    auto [cond, blkTest2] = emitLoadXParen(blkTest.move(), node->cond(), false);

    blkTest2->block()->contBranch(cond,
                                  type & C_Invert,
                                  blkThen->block(),
                                  blkDone->block());

    closure->func()->pushScope();
    auto blkThen2 = emitStmt(blkThen.move(), node->then());
    closure->func()->dropScope();

    blkThen2->block()->contStatic(blkTest2->block());

    closure->func()->dropScope();

    if (type & C_Else) {
      closure->func()->pushScope();
      auto blkOtherwise2 = emitStmt(blkOtherwise.move(), node->otherwise());
      closure->func()->dropScope();

      blkOtherwise2->block()->contStatic(blkEnd->block());
    }

    std::cerr << "/" << closure->block()->name() << std::endl;

    return blkEnd;
  }
  else {
    // NB: This stuff is order-dependent
    auto  blkThen      = closure->fork("si-then");
    auto  blkOtherwise = type & C_Else ? closure->fork("si-else") : nullptr;
    auto  blkEnd       = closure->fork("si-end");
    auto &blkDone      = type & C_Else ? blkOtherwise : blkEnd;

    closure->func()->pushScope();

    auto [cond, closure2] = emitLoadXParen(closure.move(), node->cond(), false);

    closure2->block()->contBranch(cond,
                                  type & C_Invert,
                                  blkThen->block(),
                                  blkDone->block());
    blkThen->block()->contStatic(blkEnd->block());
    if (blkOtherwise) blkOtherwise->block()->contStatic(blkEnd->block());

    closure2->func()->pushScope();
    auto blkThen2 = emitStmt(blkThen.move(), node->then());
    closure2->func()->dropScope();

    closure2->func()->dropScope();

    if (type & C_Else) {
      closure2->func()->pushScope();
      blkOtherwise = emitStmt(blkOtherwise.move(), node->otherwise());
      closure2->func()->dropScope();
    }

    return blkEnd.move();
  }
}

EMITFV(SKeyword) {
  MOVE;

  MATCH {
  case OF(Return): {
    auto [ret, closure2] = emitLoadExpr(closure.move(), node->expr());
    closure2->block()->contRet(ret);
    return closure2->fork("s-ret");
  }
  default: break;
  }

  abort();
}

EMITFL(AssignValue) {
  MOVE;
  MATCH {
  case OF(Assign): return emitLoadSAssign(closure.move(), node->assign());
  case OF(Infix): return emitLoadXInfix(closure.move(), node->infix());
  default: FAIL;
  }
}

EMITFV(Bindings, bool mut) {
  MOVE;
  MATCH {
  case OF(Bindings):
    closure = emitBindings(closure.move(), node->binds(), mut);
    NEXT;
  case OF(Binding): return emitBinding(closure.move(), node->bind(), mut);
  default: FAIL;
  }
}

EMITFV(Binding, bool mut) {
  MOVE;

  MATCH {
  case OF(BindDefault): {
    if (!mut) closure->error("'let' binding must have an initializer");

    closure->scope()->bind(node->id()->value(), mut);

    return closure;
  }
  case OF(BindInit): {
    auto [val, closure2] = emitLoadExpr(closure.move(), node->expr());

    return voidReg(closure2->emitStvar(
        "bind", closure2->scope()->bind(node->id()->value(), mut), val));
  }
  }
}

EMITFV(Decl) {
  MOVE;
  MATCH {
  case OF(Message): return emitDMsg(closure.move(), node->msg());
  case OF(Type): return emitDType(closure.move(), node->type());
  default: FAIL;
  }
}

EMITFV(DMsg) {
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
      // TODO: Actually compile message declarations
    }
    break;
  }
  default: FAIL;
  }

  return closure;
  // TODO: Introduce this message to the current scope or something.
}

EMITFV(DType) {
  MOVE;

  MATCH {
  case OF(Interface): break;
  case OF(Struct): break;
  case OF(Variant): break;
  default: FAIL;
  }

  return closure;
}

FIPraeCompiler::FIPraeCompiler(fun::FPtr<FIInputs> inputs) : m_inputs(inputs) {}

FIFunctionAtom FIPraeCompiler::compileEntryPoint(
    fun::cons_cell<const FPStmts *> args) {
  FIFunctionBody body;
  auto           fclosure = fnew<FuncClosure>(body, args.get<0>());
  auto           bclosure = flinear<BlockClosure>(fclosure, "root");

  auto bclosure2 = emitStmts(bclosure.move(), args.get<0>());

  auto [ret, bclosure3] = bclosure2->emitOp("entry", FIOpcode::Void);

  bclosure3->block()->contRet(ret);

  std::unordered_map<FIVariableAtom, fun::FPtr<FIStruct>> funcArgs;

  for (auto info : fclosure->args()->getOwned())
    funcArgs[fclosure->args()->get(info.get<0>())] = builtins::FIErrorT;

  auto id = m_inputs->assem()->funcs().intern(fnew<FIFunction>(funcArgs, body));
  m_inputs->funcs()[args.get<0>()] = id;

  return id;
}

FIFunctionAtom FIPraeCompiler::compileFunc(
    fun::cons_cell<const FPXFunc *> args) {
  FIFunctionBody body;
  auto           node     = args.get<0>();
  auto           fclosure = fnew<FuncClosure>(body, node);
  auto           bclosure = flinear<BlockClosure>(fclosure, "root");

  std::stack<const FPFuncParam *> paramStack;
  auto                            params = node->params();

  while (params) {
    MATCH_(params) {
    case OF_(params, List): params = params->params(); break;
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
      fclosure->args()->bind(pat->id()->value(), false);
      break;
    }
  }

  // TODO: Do something with this register
  auto [reg, bclosure2] = emitLoadExpr(bclosure.move(), node->expr());

  bclosure2->block()->contRet(reg);

  std::unordered_map<FIVariableAtom, fun::FPtr<FIStruct>> funcArgs;

  for (auto info : fclosure->args()->getOwned())
    funcArgs[fclosure->args()->get(info.get<0>())] = builtins::FIErrorT;

  auto id = m_inputs->assem()->funcs().intern(fnew<FIFunction>(funcArgs, body));
  m_inputs->funcs()[node] = id;

  return id;
}

std::uint32_t FIPraeCompiler::compileType(
    fun::cons_cell<const frma::FPDType *>) {
  return -1;
}
} // namespace fie
