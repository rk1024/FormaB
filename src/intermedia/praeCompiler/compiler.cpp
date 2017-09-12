#include "compiler.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <stack>

#include "util/dumpHex.hpp"

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
  closure->emit(FIOpcode::Ldfun, -1);
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
  case OF(Add): closure->emit(FIOpcode::Add); break;
  case OF(Con): closure->emit(FIOpcode::Con); break;
  case OF(Dis): closure->emit(FIOpcode::Dis); break;
  case OF(Div): closure->emit(FIOpcode::Div); break;
  case OF(Eql): closure->emit(FIOpcode::Ceq); break;
  case OF(Grt): closure->emit(FIOpcode::Cgt); break;
  case OF(Geq): closure->emit(FIOpcode::Clt).emit(FIOpcode::Inv); break;
  case OF(Lss): closure->emit(FIOpcode::Clt); break;
  case OF(Leq): closure->emit(FIOpcode::Cgt).emit(FIOpcode::Inv); break;
  case OF(Mod): closure->emit(FIOpcode::Mod); break;
  case OF(Mul): closure->emit(FIOpcode::Mul); break;
  case OF(Neq): closure->emit(FIOpcode::Ceq).emit(FIOpcode::Inv); break;
  case OF(Sub): closure->emit(FIOpcode::Sub); break;
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
  MATCH {
  case OF(Message): {
    emitLoadXPrim(closure, node->expr());

    auto sel = node->sel();

    std::ostringstream oss;
    std::uint32_t      count =
        1; // The count's already at 1 because of the message receiver

    MATCH_(sel) {
    case OF_(sel, Unary): oss << sel->tok()->toString(); break;
    case OF_(sel, Keyword): {
      std::stack<const FPMsgKeyword *> kwStack;
      const FPMsgKeywords *            kws = sel->kws();

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
        m_inputs->assem()->msgs().intern(fun::cons(count, oss.str())));
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

    if (flags & ParenFlags::Scope) closure->applyScope();
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
  case OF(Inv): closure->emit(FIOpcode::Inv); break;
  case OF(Neg): closure->emit(FIOpcode::Neg); break;
  case OF(Pos): closure->emit(FIOpcode::Pos); break;
  case OF(Member):
  default: FAIL;
  }

  if (inc) {
    closure->emit<std::int32_t>(FIOpcode::Ldci4, 1)
        .emit(inc == I_Inc ? FIOpcode::Add : FIOpcode::Sub)
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

void EMITF(FuncParams) {
  MOVE;
  MATCH {
  case OF(Empty): break;
  case OF(List): emitFuncParams(closure, node->params()); break;
  case OF(Parameters): emitFuncParams(closure, node->params()); NEXT;
  case OF(Parameter): emitFuncParam(closure, node->param()); break;
  case OF(Error):
  default: FAIL;
  }
}

void EMITF(FuncParam) {
  MOVE;
  // NB: param->id()->value() ends with a colon (i.e. 'var:' instead of 'var')
  closure->scope()->bind(
      std::string(node->id()->value(), 0, node->id()->value().size() - 1),
      false);
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

  FIOpcode op = FIOpcode::Nop;

  MATCH {
  case OF(Add): op = FIOpcode::Add; break;
  case OF(Con): op = FIOpcode::Con; break;
  case OF(Dis): op = FIOpcode::Dis; break;
  case OF(Div): op = FIOpcode::Div; break;
  case OF(Mod): op = FIOpcode::Mod; break;
  case OF(Mul): op = FIOpcode::Mul; break;
  case OF(Sub): op = FIOpcode::Sub; break;
  case OF(Assign): break;
  default: FAIL;
  }

  if (op != FIOpcode::Nop) emitLoadXMember(closure, node->memb());

  emitLoadAssignValue(closure, node->value());

  if (op != FIOpcode::Nop) closure->emit(op);

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

FIPraeCompiler::FIPraeCompiler(fun::FPtr<FIInputs> inputs) : m_inputs(inputs) {}

void FIPraeCompiler::accept(const FPStmts *node) {
  FIBytecode body;
  auto       closure = fnew<FuncClosure>(body, node);

  emitStmts(closure, node);

  closure->emit(FIOpcode::Ldvoid).emit(FIOpcode::Ret);

  /* return */ m_inputs->assem()->funcs().intern(fnew<FIFunction>(body));
}

void FIPraeCompiler::accept(const FPXFunc *node) {
  FIBytecode body;
  auto       closure = fnew<FuncClosure>(body, node);

  emitFuncParams(closure, node->params());
  emitLoadExpr(closure, node->expr());

  closure->emit(FIOpcode::Ret);

  /* return */ m_inputs->assem()->funcs().intern(fnew<FIFunction>(body));
}
}
