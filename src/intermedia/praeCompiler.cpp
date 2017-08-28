#include "praeCompiler.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <stack>

#include "util/dumpHex.hpp"

using namespace frma;

namespace fie {
std::uint32_t _FIPraeCompiler::emitLoadExprs(FuncClosure &  closure,
                                             const FPExprs *exprs,
                                             bool           tuple) {
  std::uint32_t count = 0;

  switch (exprs->alt()) {
  case FPExprs::Empty: return 0;
  case FPExprs::Exprs:
    count = emitLoadExprs(closure, exprs->exprs(), false);
    [[clang::fallthrough]];
  case FPExprs::Expr:
    if (emitLoadExpr(closure, exprs->expr())) ++count;

    if (count > 1 && tuple) {
      closure.emit(FIOpcode::Tpl, count);

      return -1;
    }

    return count;
  }

  assert(false);
}

bool _FIPraeCompiler::emitLoadExpr(FuncClosure &closure, const FPExpr *expr) {
  switch (expr->alt()) {
  case FPExpr::Control: return emitLoadXControl(closure, expr->ctl());
  case FPExpr::Function: return emitLoadXFunc(closure, expr->func());
  case FPExpr::Infix: return emitLoadXInfix(closure, expr->infix());
  default: assert(false);
  }
}

bool _FIPraeCompiler::emitLoadLBoolean(FuncClosure &     closure,
                                       const FPLBoolean *boolean) {
  switch (boolean->alt()) {
  case FPLBoolean::False: closure.emit<std::int32_t>(FIOpcode::Ldci4, 0); break;
  case FPLBoolean::True: closure.emit<std::int32_t>(FIOpcode::Ldci4, 1); break;
  default: assert(false);
  }

  return true;
}

bool _FIPraeCompiler::emitLoadXBlock(FuncClosure &   closure,
                                     const FPXBlock *block) {
  switch (block->alt()) {
  case FPXBlock::Error: break;
  case FPXBlock::Block:
    emitStmts(closure, block->stmts());
    return false; // TODO: This should sometimes return stuff maybe
  }

  assert(false);
}

bool _FIPraeCompiler::emitLoadXControl(FuncClosure &     closure,
                                       const FPXControl *ctl) {
  bool invert = false;

  switch (ctl->alt()) {
  case FPXControl::If: break;
  case FPXControl::Unless: invert = true; break;
  default: assert(false);
  }

  auto lblElse = closure.beginLabel();

  if (!emitLoadXParen(closure, ctl->cond(), true))
    throw std::runtime_error("condition must load value");

  closure.emit(
      FIInstruction::brLbl(invert ? FIOpcode::Bnz : FIOpcode::Bez, lblElse));

  bool thenLoad = emitLoadExpr(closure, ctl->then());

  closure.label(lblElse);

  if (emitLoadExpr(closure, ctl->otherwise()) != thenLoad)
    throw std::runtime_error("not all paths load a value");

  return thenLoad;
}

bool _FIPraeCompiler::emitLoadXFunc(FuncClosure &closure, const FPXFunc *func) {
  FIBytecode  body;
  FuncClosure closure2(closure.assem(), body);

  emitLoadExpr(closure2, func->expr());

  if (closure2.body()->instructions.empty() ||
      closure2.body()
              ->instructions.at(closure2.body()->instructions.size() - 1)
              .op != FIOpcode::Ret)
    closure2.emit(FIOpcode::Ret);

  FIFunction fiFunc(body);

  auto tok = closure.assem()->funcs().intern(fiFunc);

  closure.emit(FIOpcode::Ldfun, tok);

  return true;
}

bool _FIPraeCompiler::emitLoadXInfix(FuncClosure &   closure,
                                     const FPXInfix *infix) {
  if (infix->alt() == FPXInfix::Unary)
    return emitLoadXUnary(closure, infix->unary());

  if (!(infix->alt() == FPXInfix::Mod ?
            emitLoadXUnary(closure, infix->unary()) :
            emitLoadXInfix(closure, infix->infixl())))
    throw std::runtime_error("first operand must load value");

  if (!emitLoadXInfix(closure, infix->infixr()))
    throw std::runtime_error("second operand must load value");

  switch (infix->alt()) {
  case FPXInfix::Add: closure.emit(FIOpcode::Add); break;
  case FPXInfix::Con: closure.emit(FIOpcode::Con); break;
  case FPXInfix::Dis: closure.emit(FIOpcode::Dis); break;
  case FPXInfix::Div: closure.emit(FIOpcode::Div); break;
  case FPXInfix::Eql: closure.emit(FIOpcode::Ceq); break;
  case FPXInfix::Grt: closure.emit(FIOpcode::Cgt); break;
  case FPXInfix::Geq: closure.emit(FIOpcode::Clt).emit(FIOpcode::Inv); break;
  case FPXInfix::Lss: closure.emit(FIOpcode::Clt); break;
  case FPXInfix::Leq: closure.emit(FIOpcode::Cgt).emit(FIOpcode::Inv); break;
  case FPXInfix::Mod: closure.emit(FIOpcode::Mod); break;
  case FPXInfix::Mul: closure.emit(FIOpcode::Mul); break;
  case FPXInfix::Neq: closure.emit(FIOpcode::Ceq).emit(FIOpcode::Inv); break;
  case FPXInfix::Sub: closure.emit(FIOpcode::Sub); break;
  default: assert(false);
  }

  return true;
}

bool _FIPraeCompiler::emitLoadXMember(FuncClosure &    closure,
                                      const FPXMember *memb) {
  switch (memb->alt()) {
  case FPXMember::Member: /* emitLoadXMember(closure, memb->memb()); */ break;
  case FPXMember::Primary: return emitLoadXPrim(closure, memb->prim());
  }

  assert(false);
}

bool _FIPraeCompiler::emitLoadXParen(FuncClosure &   closure,
                                     const FPXParen *paren,
                                     bool /* scoped */) {
  switch (paren->alt()) {
  case FPXParen::Error: break;
  case FPXParen::Paren: return emitLoadXParen(closure, paren->paren());
  case FPXParen::Tuple: return emitLoadExprs(closure, paren->exprs());
  case FPXParen::Where:
    emitSBind(closure, paren->bind());
    return emitLoadExpr(closure, paren->expr());
  }

  assert(false);
}

bool _FIPraeCompiler::emitLoadXPrim(FuncClosure &closure, const FPXPrim *prim) {
  switch (prim->alt()) {
  case FPXPrim::Block: return emitLoadXBlock(closure, prim->block());
  case FPXPrim::Boolean: return emitLoadLBoolean(closure, prim->boolean());
  case FPXPrim::DQLiteral: break;
  case FPXPrim::Identifier: closure.emit(FIOpcode::PH_LdVar); return true;
  case FPXPrim::Message: closure.emit(FIOpcode::PH_Msg); return true;
  case FPXPrim::Numeric: return emitLoadLNumeric(closure, prim->numeric());
  case FPXPrim::Parens: return emitLoadXParen(closure, prim->paren());
  case FPXPrim::SQLiteral: break;
  }

  assert(false);
}

bool _FIPraeCompiler::emitLoadXUnary(FuncClosure &   closure,
                                     const FPXUnary *unary) {
  const std::uint8_t I_Inc = 1, I_Dec = 2;

  if (unary->alt() == FPXUnary::Member)
    return emitLoadXMember(closure, unary->memb());

  if (!emitLoadXUnary(closure, unary->unary()))
    throw std::runtime_error("unary operand must laod value");

  std::uint8_t inc = 0;

  switch (unary->alt()) {
  case FPXUnary::Dec: inc = I_Dec; break;
  case FPXUnary::Inc: inc = I_Inc; break;
  case FPXUnary::Inv: closure.emit(FIOpcode::Inv); break;
  case FPXUnary::Neg: closure.emit(FIOpcode::Neg); break;
  case FPXUnary::Pos: closure.emit(FIOpcode::Pos); break;
  default: assert(false);
  }

  if (inc) {
    closure.emit<std::int32_t>(FIOpcode::Ldci4, 1)
        .emit(inc == I_Inc ? FIOpcode::Add : FIOpcode::Sub)
        .emit(FIOpcode::Dup)
        .emit(FIOpcode::PH_Bind);
  }

  return true;
}

void _FIPraeCompiler::emitStmts(FuncClosure &closure, const FPStmts *stmts) {
  switch (stmts->alt()) {
  case FPStmts::Empty: return;
  case FPStmts::Statements:
    emitStmts(closure, stmts->stmts());
    [[clang::fallthrough]];
  case FPStmts::Statement: emitStmt(closure, stmts->stmt()); return;
  default: assert(false);
  }
}

void _FIPraeCompiler::emitStmt(FuncClosure &closure, const FPStmt *stmt) {
  switch (stmt->alt()) {
  case FPStmt::Assign: assert(false);
  case FPStmt::Bind: emitSBind(closure, stmt->bind()); return;
  case FPStmt::Control: emitSControl(closure, stmt->ctl()); return;
  case FPStmt::Error: assert(false);
  case FPStmt::NonSemiExpr:
  case FPStmt::SemiExpr:
    if (emitLoadExpr(closure, stmt->expr())) closure.emit(FIOpcode::Pop);

    return;
  default: assert(false);
  }
}

void _FIPraeCompiler::emitSBind(FuncClosure &closure, const FPSBind *bind) {
  switch (bind->alt()) {
  case FPSBind::Let: emitBindings(closure, bind->binds(), false); return;
  case FPSBind::Var: emitBindings(closure, bind->binds(), true); return;
  }

  assert(false);
}

void _FIPraeCompiler::emitSControl(FuncClosure &     closure,
                                   const FPSControl *ctl) {
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
  case FPSControl::UntilElse:
    type = C_Loop | C_Invert | C_Else;
    break;

  // case FPSControl::DoWhile: type      = C_Do | C_Loop; break;
  // case FPSControl::DoWhileElse: type  = C_Do | C_Loop | C_Else; break;
  // case FPSControl::DoUntil: type      = C_Do | C_Loop | C_Invert; break;
  // case FPSControl::DoUntilElse: type  = C_Do | C_Loop | C_Invert | C_Else;
  // break;
  default: assert(false);
  }

  if (type & C_Loop) {
    std::uint16_t lblDo = closure.beginLabel(), lblTest = closure.beginLabel(),
                  lblBreak = closure.beginLabel();

    if (!(type & C_Do))
      closure.emit(FIInstruction::brLbl(FIOpcode::Br, lblTest));

    closure.label(lblDo);

    emitStmt(closure, ctl->then());

    closure.label(lblTest);

    if (!emitLoadXParen(closure, ctl->cond(), !(type & C_Do)))
      throw std::runtime_error("condition must load value");

    closure.emit(FIInstruction::brLbl(
        type & C_Invert ? FIOpcode::Bez : FIOpcode::Bnz, lblDo));

    if (type & C_Else) emitStmt(closure, ctl->otherwise());

    closure.label(lblBreak);
  } else {
    std::uint16_t lblElse = closure.beginLabel();

    if (!emitLoadXParen(closure, ctl->cond(), true))
      throw std::runtime_error("condition must load value");

    closure.emit(FIInstruction::brLbl(
        type & C_Invert ? FIOpcode::Bnz : FIOpcode::Bez, lblElse));

    emitStmt(closure, ctl->then());

    closure.label(lblElse);

    if (type & C_Else) emitStmt(closure, ctl->otherwise());
  }
}

void _FIPraeCompiler::emitBindings(FuncClosure &     closure,
                                   const FPBindings *binds,
                                   bool              mut) {
  switch (binds->alt()) {
  case FPBindings::Bindings:
    emitBindings(closure, binds->binds(), mut);
    [[clang::fallthrough]];
  case FPBindings::Binding: emitBinding(closure, binds->bind(), mut); return;
  }

  assert(false);
}

void _FIPraeCompiler::emitBinding(FuncClosure &    closure,
                                  const FPBinding *bind,
                                  bool) {
  if (!emitLoadExpr(closure, bind->expr()))
    throw std::runtime_error("bind expression must load value");

  closure.emit(FIOpcode::PH_Bind); // TODO: Actually bind this
}

// FIFunction _FIPraeCompiler::compile(const FPXFunc *func) {
//   FIBytecode body;

//   FuncClosure closure(body);

//   emitFunc(closure, func);

//   return FIFunction(body);
// }

std::vector<std::pair<const FPBlock *, std::uint16_t>> _FIPraeCompiler::
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

void _FIPraeCompiler::dump(std::ostream &os) const {
  for (std::size_t i = 0; i < m_assems.size(); ++i) {
    auto assem = m_assems.value(i);

    os << "Assembly " << i << ":" << std::endl;

    for (std::uint16_t j = 0; j < assem->funcs().size(); ++j) {
      os << "  Function " << i << "." << j << ":" << std::endl;

      auto func = assem->funcs().value(j);

      auto body = func->body();

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

        case FIOpcode::Ldstr: os << "ldstr " << fun::dumpHex(ins.u2); break;
        case FIOpcode::Ldfun: os << "ldfun " << fun::dumpHex(ins.u2); break;

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
        case FIOpcode::PH_LdVar: os << "<LD_VAR>"; break;
        case FIOpcode::PH_Msg: os << "<MSG>"; break;

        default: assert(false);
        }

        os << std::endl;
      }
    }
  }
}
}
