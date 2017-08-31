#include "praeCompiler.hpp"
#include "praeCompilerClosures.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <stack>

#include "util/dumpHex.hpp"

using namespace frma;
using namespace fie::pc;

#define _MOVE_2(id) ctx##id
#define _MOVE_1(closure, to, id) auto _MOVE_2(id) = closure->move(to);
#define MOVE_(closure, to) _MOVE_1(closure, to, __COUNTER__)
#define MOVE(to) MOVE_(closure, to);

namespace fie {
std::uint32_t FIPraeCompiler::emitLoadExprs(fun::FPtr<FuncClosure> closure,
                                            const FPExprs *        exprs,
                                            bool                   tuple) {
  MOVE(exprs)
  std::uint32_t count = 0;

  switch (exprs->alt()) {
  case FPExprs::Empty: return 0;
  case FPExprs::Exprs:
    count = emitLoadExprs(closure, exprs->exprs(), false);
    [[clang::fallthrough]];
  case FPExprs::Expr:
    if (emitLoadExpr(closure, exprs->expr())) ++count;

    if (count > 1 && tuple) {
      closure->emit(FIOpcode::Tpl, count);

      return -1;
    }

    return count;
  }

  assert(false);
}

bool FIPraeCompiler::emitLoadExpr(fun::FPtr<FuncClosure> closure,
                                  const FPExpr *         expr) {
  MOVE(expr)
  switch (expr->alt()) {
  case FPExpr::Control: return emitLoadXControl(closure, expr->ctl());
  case FPExpr::Function: return emitLoadXFunc(closure, expr->func());
  case FPExpr::Infix: return emitLoadXInfix(closure, expr->infix());
  default: assert(false);
  }
}

bool FIPraeCompiler::emitLoadLBoolean(fun::FPtr<FuncClosure> closure,
                                      const FPLBoolean *     boolean) {
  MOVE(boolean)
  switch (boolean->alt()) {
  case FPLBoolean::False:
    closure->emit<std::int32_t>(FIOpcode::Ldci4, 0);
    break;
  case FPLBoolean::True: closure->emit<std::int32_t>(FIOpcode::Ldci4, 1); break;
  default: assert(false);
  }

  return true;
}

bool FIPraeCompiler::emitLoadXBlock(fun::FPtr<FuncClosure> closure,
                                    const FPXBlock *       block) {
  MOVE(block)
  switch (block->alt()) {
  case FPXBlock::Error: break;
  case FPXBlock::Block:
    emitStmts(closure, block->stmts());
    return false; // TODO: This should sometimes return stuff maybe
  }

  assert(false);
}

bool FIPraeCompiler::emitLoadXControl(fun::FPtr<FuncClosure> closure,
                                      const FPXControl *     ctl) {
  MOVE(ctl)
  bool invert = false;

  switch (ctl->alt()) {
  case FPXControl::If: break;
  case FPXControl::Unless: invert = true; break;
  default: assert(false);
  }

  auto lblElse = closure->beginLabel();

  if (!emitLoadXParen(closure, ctl->cond(), true)) {
    MOVE(ctl->cond())
    closure->error("condition must load value");
  }

  closure->emit(
      FIInstruction::brLbl(invert ? FIOpcode::Bnz : FIOpcode::Bez, lblElse));

  bool thenLoad = emitLoadExpr(closure, ctl->then());

  closure->label(lblElse);

  if (emitLoadExpr(closure, ctl->otherwise()) != thenLoad)
    closure->error("not all paths load a value");

  return thenLoad;
}

bool FIPraeCompiler::emitLoadXFunc(fun::FPtr<FuncClosure> closure,
                                   const FPXFunc *        func) {
  MOVE(func)
  FIBytecode body;
  auto       closure2 = fnew<FuncClosure>(closure->assem(), body, func);

  emitFuncParams(closure2, func->params());

  emitLoadExpr(closure2, func->expr());

  if (closure2->body()->instructions.empty() ||
      closure2->body()
              ->instructions.at(closure2->body()->instructions.size() - 1)
              .op != FIOpcode::Ret)
    closure2->emit(FIOpcode::Ret);

  auto fiFunc = fnew<FIFunction>(body);
  auto tok    = closure->assem()->funcs().intern(fiFunc);

  closure->emit(FIOpcode::Ldfun, tok);

  return true;
}

bool FIPraeCompiler::emitLoadXInfix(fun::FPtr<FuncClosure> closure,
                                    const FPXInfix *       infix) {
  MOVE(infix)
  if (infix->alt() == FPXInfix::Unary)
    return emitLoadXUnary(closure, infix->unary());

  if (!(infix->alt() == FPXInfix::Mod ?
            emitLoadXUnary(closure, infix->unary()) :
            emitLoadXInfix(closure, infix->infixl())))
    closure->error("first operand must load value");

  if (!emitLoadXInfix(closure, infix->infixr()))
    closure->error("second operand must load value");

  switch (infix->alt()) {
  case FPXInfix::Add: closure->emit(FIOpcode::Add); break;
  case FPXInfix::Con: closure->emit(FIOpcode::Con); break;
  case FPXInfix::Dis: closure->emit(FIOpcode::Dis); break;
  case FPXInfix::Div: closure->emit(FIOpcode::Div); break;
  case FPXInfix::Eql: closure->emit(FIOpcode::Ceq); break;
  case FPXInfix::Grt: closure->emit(FIOpcode::Cgt); break;
  case FPXInfix::Geq: closure->emit(FIOpcode::Clt).emit(FIOpcode::Inv); break;
  case FPXInfix::Lss: closure->emit(FIOpcode::Clt); break;
  case FPXInfix::Leq: closure->emit(FIOpcode::Cgt).emit(FIOpcode::Inv); break;
  case FPXInfix::Mod: closure->emit(FIOpcode::Mod); break;
  case FPXInfix::Mul: closure->emit(FIOpcode::Mul); break;
  case FPXInfix::Neq: closure->emit(FIOpcode::Ceq).emit(FIOpcode::Inv); break;
  case FPXInfix::Sub: closure->emit(FIOpcode::Sub); break;
  default: assert(false);
  }

  return true;
}

bool FIPraeCompiler::emitLoadXMember(fun::FPtr<FuncClosure> closure,
                                     const FPXMember *      memb) {
  MOVE(memb)
  switch (memb->alt()) {
  case FPXMember::Member: /* emitLoadXMember(closure, memb->memb()); */ break;
  case FPXMember::Primary: return emitLoadXPrim(closure, memb->prim());
  }

  assert(false);
}

bool FIPraeCompiler::emitLoadXParen(fun::FPtr<FuncClosure> closure,
                                    const FPXParen *       paren,
                                    bool                   scoped) {
  MOVE(paren)
  switch (paren->alt()) {
  case FPXParen::Error: break;
  case FPXParen::Paren: return emitLoadXParen(closure, paren->paren());
  case FPXParen::Tuple: return emitLoadExprs(closure, paren->exprs());
  case FPXParen::Where:
    if (scoped) closure->beginScope();
    emitSBind(closure, paren->bind());
    auto ret = emitLoadExpr(closure, paren->expr());
    if (scoped) closure->endScope();
    return ret;
  }

  assert(false);
}

bool FIPraeCompiler::emitLoadXPrim(fun::FPtr<FuncClosure> closure,
                                   const FPXPrim *        prim) {
  MOVE(prim)
  switch (prim->alt()) {
  case FPXPrim::Block: return emitLoadXBlock(closure, prim->block());
  case FPXPrim::Boolean: return emitLoadLBoolean(closure, prim->boolean());
  case FPXPrim::DQLiteral: break;
  case FPXPrim::Identifier:
    closure->emit(FIOpcode::Ldvar,
                  closure->scope()->get(prim->tok()->value(), false));
    return true;
  case FPXPrim::Message: closure->emit(FIOpcode::PH_Msg); return true;
  case FPXPrim::Numeric: return emitLoadLNumeric(closure, prim->numeric());
  case FPXPrim::Parens: return emitLoadXParen(closure, prim->paren());
  case FPXPrim::SQLiteral: break;
  }

  assert(false);
}

bool FIPraeCompiler::emitLoadXUnary(fun::FPtr<FuncClosure> closure,
                                    const FPXUnary *       unary) {
  MOVE(unary)
  const std::uint8_t I_Inc = 1, I_Dec = 2;

  if (unary->alt() == FPXUnary::Member)
    return emitLoadXMember(closure, unary->memb());

  if (!emitLoadXUnary(closure, unary->unary()))
    closure->error("unary operand must laod value");

  std::uint8_t inc = 0;

  switch (unary->alt()) {
  case FPXUnary::Dec: inc = I_Dec; break;
  case FPXUnary::Inc: inc = I_Inc; break;
  case FPXUnary::Inv: closure->emit(FIOpcode::Inv); break;
  case FPXUnary::Neg: closure->emit(FIOpcode::Neg); break;
  case FPXUnary::Pos: closure->emit(FIOpcode::Pos); break;
  default: assert(false);
  }

  if (inc) {
    closure->emit<std::int32_t>(FIOpcode::Ldci4, 1)
        .emit(inc == I_Inc ? FIOpcode::Add : FIOpcode::Sub)
        .emit(FIOpcode::Dup)
        .emit(FIOpcode::PH_Bind);
  }

  return true;
}

void FIPraeCompiler::emitFuncParams(fun::FPtr<FuncClosure> closure,
                                    const FPFuncParams *   params) {
  MOVE(params)
  switch (params->alt()) {
  case FPFuncParams::Empty: break;
  case FPFuncParams::Error: assert(false);
  case FPFuncParams::List: emitFuncParams(closure, params->params()); break;
  case FPFuncParams::Parameters:
    emitFuncParams(closure, params->params());
    [[clang::fallthrough]];
  case FPFuncParams::Parameter: emitFuncParam(closure, params->param()); break;
  default: assert(false);
  }
}

void FIPraeCompiler::emitFuncParam(fun::FPtr<FuncClosure> closure,
                                   const FPFuncParam *    param) {
  MOVE(param)
  // NB: param->id()->value() ends with a colon (i.e. 'var:' instead of 'var')
  closure->scope()->bind(
      std::string(param->id()->value(), 0, param->id()->value().size() - 1),
      false);
}

void FIPraeCompiler::emitStmts(fun::FPtr<FuncClosure> closure,
                               const FPStmts *        stmts) {
  MOVE(stmts)
  switch (stmts->alt()) {
  case FPStmts::Empty: break;
  case FPStmts::Statements:
    emitStmts(closure, stmts->stmts());
    [[clang::fallthrough]];
  case FPStmts::Statement: emitStmt(closure, stmts->stmt()); break;
  default: assert(false);
  }
}

void FIPraeCompiler::emitStmt(fun::FPtr<FuncClosure> closure,
                              const FPStmt *         stmt) {
  MOVE(stmt)
  switch (stmt->alt()) {
  case FPStmt::Assign: assert(false);
  case FPStmt::Bind: emitSBind(closure, stmt->bind()); break;
  case FPStmt::Control: emitSControl(closure, stmt->ctl()); break;
  case FPStmt::Error: assert(false);
  case FPStmt::NonSemiExpr:
  case FPStmt::SemiExpr:
    if (emitLoadExpr(closure, stmt->expr())) closure->emit(FIOpcode::Pop);
    break;
  default: assert(false);
  }
}

void FIPraeCompiler::emitSBind(fun::FPtr<FuncClosure> closure,
                               const FPSBind *        bind) {
  MOVE(bind)
  switch (bind->alt()) {
  case FPSBind::Let: emitBindings(closure, bind->binds(), false); break;
  case FPSBind::Var: emitBindings(closure, bind->binds(), true); break;
  default: assert(false);
  }
}

void FIPraeCompiler::emitSControl(fun::FPtr<FuncClosure> closure,
                                  const FPSControl *     ctl) {
  MOVE(ctl)
  const std::uint8_t C_Else = 0x1, C_Invert = 0x2, C_Loop = 0x4, C_Do = 0x8;

  std::uint8_t type = 0;

  switch (ctl->alt()) {
  case FPSControl::If: break;
  case FPSControl::IfElse: type     = C_Else; break;
  case FPSControl::Unless: type     = C_Invert; break;
  case FPSControl::UnlessElse: type = C_Invert | C_Else; break;
  case FPSControl::While: type      = C_Loop; break;
  case FPSControl::WhileElse: type  = C_Loop | C_Else; break;
  case FPSControl::Until: type      = C_Loop | C_Invert; break;
  case FPSControl::UntilElse: type  = C_Loop | C_Invert | C_Else; break;
  default: assert(false);
  }

  if (type & C_Loop) {
    std::uint16_t lblDo    = closure->beginLabel(),
                  lblTest  = closure->beginLabel(),
                  lblBreak = closure->beginLabel();

    if (!(type & C_Do))
      closure->emit(FIInstruction::brLbl(FIOpcode::Br, lblTest));

    closure->label(lblDo);

    emitStmt(closure, ctl->then());

    closure->label(lblTest);

    if (!emitLoadXParen(closure, ctl->cond(), !(type & C_Do)))
      closure->error("condition must load value");

    closure->emit(FIInstruction::brLbl(
        type & C_Invert ? FIOpcode::Bez : FIOpcode::Bnz, lblDo));

    if (type & C_Else) emitStmt(closure, ctl->otherwise());

    closure->label(lblBreak);
  } else {
    std::uint16_t lblElse = closure->beginLabel();

    if (!emitLoadXParen(closure, ctl->cond(), true))
      closure->error("condition must load value");

    closure->emit(FIInstruction::brLbl(
        type & C_Invert ? FIOpcode::Bnz : FIOpcode::Bez, lblElse));

    emitStmt(closure, ctl->then());

    closure->label(lblElse);

    if (type & C_Else) emitStmt(closure, ctl->otherwise());
  }
}

void FIPraeCompiler::emitBindings(fun::FPtr<FuncClosure> closure,
                                  const FPBindings *     binds,
                                  bool                   mut) {
  MOVE(binds)
  switch (binds->alt()) {
  case FPBindings::Bindings:
    emitBindings(closure, binds->binds(), mut);
    [[clang::fallthrough]];
  case FPBindings::Binding: emitBinding(closure, binds->bind(), mut); break;
  default: assert(false);
  }
}

void FIPraeCompiler::emitBinding(fun::FPtr<FuncClosure> closure,
                                 const FPBinding *      bind,
                                 bool                   mut) {
  MOVE(bind)
  if (!emitLoadExpr(closure, bind->expr()))
    closure->error("bind expression must load value");

  closure->emit<std::uint32_t>(
      FIOpcode::Stvar, closure->scope()->bind(bind->id()->value(), mut));
}

std::uint16_t FIPraeCompiler::compileEntryPoint(
    decltype(m_assems.emplace()) assem, const FPStmts *stmts) {
  FIBytecode body;
  auto       closure = fnew<FuncClosure>(m_assems.value(assem), body, stmts);

  emitStmts(closure, stmts);

  if (closure->body()->instructions.empty() ||
      closure->body()
              ->instructions.at(closure->body()->instructions.size() - 1)
              .op != FIOpcode::Ret)
    closure->emit(FIOpcode::Ret);

  return m_assems.value(assem)->funcs().emplace(fnew<FIFunction>(body));
}


std::vector<std::pair<const FPBlock *, std::uint16_t>> FIPraeCompiler::
    compileBlocks(std::size_t assem, const FPrims *prims) {
  std::stack<const FPrim *> stack;

  while (prims) {
    switch (prims->alt()) {
    case FPrims::Empty: prims = nullptr; break;
    case FPrims::Primaries:
      stack.push(prims->prim());
      prims = prims->prims();
      break;
    case FPrims::Primary:
      stack.push(prims->prim());
      prims = nullptr;
      break;
    }
  }

  std::vector<std::pair<const FPBlock *, std::uint16_t>> ret;

  while (stack.size()) {
    if (stack.top()->alt() == FPrim::PraeBlock)
      ret.emplace_back(
          stack.top()->praeblk(),
          compileEntryPoint(assem, stack.top()->praeblk()->stmts()));
    stack.pop();
  }

  return ret;
}

void FIPraeCompiler::dump(std::ostream &os) const {
  for (std::size_t i = 0; i < m_assems.size(); ++i) {
    auto assem = m_assems.value(i);

    os << "Assembly " << i << ":" << std::endl;

    for (std::uint16_t j = 0; j < assem->funcs().size(); ++j) {
      auto func = assem->funcs().value(j);
      auto body = func->body();

      os << "  Function " << i << "." << j << " (" << body.instructions.size()
         << "):" << std::endl;

      for (std::size_t k = 0; k < body.instructions.size(); ++k) {
        os << "    ";

        for (std::uint16_t l = 0; l < body.labels.size(); ++l) {
          if (body.labels.at(l).pos == k)
            os << body.labels.at(l).name << ":" << std::endl << "    ";
        }

        if (body.labels.size()) os << "  ";

        auto ins = body.instructions.at(k);

        switch (ins.op) {
        case FIOpcode::Nop: os << "nop"; break;

        case FIOpcode::Dup: os << "dup"; break;
        case FIOpcode::Pop: os << "pop"; break;
        case FIOpcode::Ret: os << "ret"; break;

        case FIOpcode::Add: os << "add"; break;
        case FIOpcode::Sub: os << "sub"; break;
        case FIOpcode::Mul: os << "mul"; break;
        case FIOpcode::Div: os << "div"; break;
        case FIOpcode::Mod: os << "mod"; break;

        case FIOpcode::Neg: os << "neg"; break;
        case FIOpcode::Pos: os << "pos"; break;

        case FIOpcode::Ceq: os << "ceq"; break;
        case FIOpcode::Cgt: os << "cgt"; break;
        case FIOpcode::Cgtu: os << "cgtu"; break;
        case FIOpcode::Clt: os << "clt"; break;
        case FIOpcode::Cltu: os << "cltu"; break;

        case FIOpcode::Con: os << "con"; break;
        case FIOpcode::Dis: os << "dis"; break;

        case FIOpcode::Inv: os << "inv"; break;

        case FIOpcode::Br:
          os << "br " << (ins.br.lbl ? body.labels.at(ins.br.id).name :
                                       std::to_string(ins.br.addr));
          break;
        case FIOpcode::Bez:
          os << "bez " << (ins.br.lbl ? body.labels.at(ins.br.id).name :
                                        std::to_string(ins.br.addr));
          break;
        case FIOpcode::Bnz:
          os << "bnz " << (ins.br.lbl ? body.labels.at(ins.br.id).name :
                                        std::to_string(ins.br.addr));
          break;

        case FIOpcode::Ldci4: os << "ldci4 " << ins.i4; break;
        case FIOpcode::Ldci8: os << "ldci8 " << ins.i8; break;
        case FIOpcode::Ldcr4: os << "ldcr4 " << ins.r4; break;
        case FIOpcode::Ldcr8: os << "ldcr8 " << ins.r8; break;

        case FIOpcode::Ldnil: os << "ldnil"; break;

        case FIOpcode::Ldvar: os << "ldvar " << body.vars.value(ins.u4); break;

        case FIOpcode::Ldstr: os << "ldstr " << fun::dumpHex(ins.u2); break;
        case FIOpcode::Ldfun: os << "ldfun " << fun::dumpHex(ins.u2); break;

        case FIOpcode::Stvar: os << "stvar " << body.vars.value(ins.u4); break;

        case FIOpcode::Cvi1: os << "cvi1"; break;
        case FIOpcode::Cvi2: os << "cvi2"; break;
        case FIOpcode::Cvi4: os << "cvi4"; break;
        case FIOpcode::Cvi8: os << "cvi8"; break;
        case FIOpcode::Cvu1: os << "cvu1"; break;
        case FIOpcode::Cvu2: os << "cvu2"; break;
        case FIOpcode::Cvu4: os << "cvu4"; break;
        case FIOpcode::Cvu8: os << "cvu8"; break;
        case FIOpcode::Cvr4: os << "cvr4"; break;
        case FIOpcode::Cvr8: os << "cvr8"; break;

        case FIOpcode::Tpl: os << "tpl " << fun::dumpHex(ins.u4); break;

        case FIOpcode::PH_Bind: os << "<BIND>"; break;
        case FIOpcode::PH_LdInt: os << "<LD_INT>"; break;
        case FIOpcode::PH_LdReal: os << "<LD_REAL>"; break;
        case FIOpcode::PH_Msg: os << "<MSG>"; break;

        default: assert(false);
        }

        os << std::endl;
      }
    }
  }
}
}
