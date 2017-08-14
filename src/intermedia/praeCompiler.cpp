#include "praeCompiler.hpp"

#include <functional>

#include "util/atomStore.hpp"

using namespace frma;

namespace fie {
class _FIPraeCompiler::AssemblyClosure {
  fun::AtomStore<std::string> m_atoms;

public:
  inline auto atoms() -> decltype(m_atoms) & { return m_atoms; }
};

class _FIPraeCompiler::FuncClosure {
  std::vector<std::int8_t> *m_body;

public:
  FuncClosure(std::vector<std::int8_t> &body) : m_body(&body) {}

  inline void emit(std::int8_t byte) { m_body->push_back(byte); }

  inline void emit(FIOpcode op) {
    m_body->push_back(static_cast<std::int8_t>(op));
  }

  template <typename T>
  inline typename std::enable_if<std::is_fundamental<T>::value>::type emit(
      T val) {
    std::int8_t bytes[sizeof(T) / sizeof(std::int8_t)] = val;

    for (auto byte : bytes) emit(byte);
  }
};

// void _FIPraeCompiler::emitLoadCtl(FuncClosure &closure, const FPXControl
// *ctl) {
//   switch (ctl->alt()) {
//   case FPXControl::If: break;
//   case FPXControl::IfElse: break;
//   case FPXControl::IfExpr: break;
//   case FPXControl::IfElseExpr: break;
//   case FPXControl::Unless: break;
//   case FPXControl::UnlessElse: break;
//   case FPXControl::UnlessExpr: break;
//   case FPXControl::UnlessElseExpr: break;
//   case FPXControl::While: break;
//   case FPXControl::WhileElse: break;
//   case FPXControl::WhileExpr: break;
//   case FPXControl::WhileElseExpr: break;
//   case FPXControl::Until: break;
//   case FPXControl::UntilElse: break;
//   case FPXControl::UntilExpr: break;
//   case FPXControl::UntilElseExpr: break;
//   }
// }

void _FIPraeCompiler::emitLoadExpr(FuncClosure &closure, const FPExpr *expr) {
  switch (expr->alt()) {
  case FPExpr::Control: emitLoadXCtl(closure, expr->ctl()); break;
  case FPExpr::Function: break;
  case FPExpr::Infix: emitLoadXInfix(closure, expr->infix()); break;
  }
}

void _FIPraeCompiler::emitLoadLBoolean(FuncClosure &, const FPLBoolean *) {}

void _FIPraeCompiler::emitLoadLNumeric(FuncClosure &,
                                       const FPLNumeric *numeric) {
  switch (numeric->alt()) {
  case FPLNumeric::Hex: break;
  case FPLNumeric::Dec: break;
  case FPLNumeric::Oct: break;
  case FPLNumeric::Bin: break;
  case FPLNumeric::Float: break;
  }
}

void _FIPraeCompiler::emitLoadXCtl(FuncClosure &, const FPXControl *ctl) {
  switch (ctl->alt()) {
  // case FPXControl::If: break;
  case FPXControl::If /*Else*/:
    break;
  // case FPXControl::Unless: break;
  case FPXControl::Unless /*Else*/: break;
  }
}

void _FIPraeCompiler::emitLoadXInfix(FuncClosure &   closure,
                                     const FPXInfix *infix) {
  if (infix->alt() == FPXInfix::Unary) {
    emitLoadXUnary(closure, infix->unary());
    return;
  }

  if (infix->alt() == FPXInfix::Mod)
    emitLoadXUnary(closure, infix->unary());
  else
    emitLoadXInfix(closure, infix->infixl());

  emitLoadXInfix(closure, infix->infixr());

  switch (infix->alt()) {
  case FPXInfix::Add: break;
  case FPXInfix::Conjunct: break;
  case FPXInfix::Disjunct: break;
  case FPXInfix::Div: break;
  case FPXInfix::Equal: break;
  case FPXInfix::Greater: break;
  case FPXInfix::GreaterEq: break;
  case FPXInfix::Less: break;
  case FPXInfix::LessEq: break;
  case FPXInfix::Mod: break;
  case FPXInfix::Mul: break;
  case FPXInfix::NotEqual: break;
  case FPXInfix::Sub: break;
  case FPXInfix::Unary: break;
  }
}

void _FIPraeCompiler::emitLoadXMember(FuncClosure &    closure,
                                      const FPXMember *memb) {
  switch (memb->alt()) {
  case FPXMember::Member: /* emitLoadXMember(closure, memb->memb()); */ break;
  case FPXMember::Primary: emitLoadXPrim(closure, memb->prim()); break;
  }
}

void _FIPraeCompiler::emitLoadXPrim(FuncClosure &closure, const FPXPrim *prim) {
  switch (prim->alt()) {
  case FPXPrim::Block: break;
  case FPXPrim::Boolean: emitLoadLBoolean(closure, prim->boolean()); break;
  case FPXPrim::DQLiteral: break;
  case FPXPrim::Identifier: break;
  case FPXPrim::Message: break;
  case FPXPrim::Numeric: emitLoadLNumeric(closure, prim->numeric()); break;
  case FPXPrim::Parens: break;
  case FPXPrim::SQLiteral: break;
  }
}

void _FIPraeCompiler::emitLoadXUnary(FuncClosure &   closure,
                                     const FPXUnary *unary) {
  if (unary->alt() == FPXUnary::Member) {
    emitLoadXMember(closure, unary->memb());
    return;
  }

  emitLoadXUnary(closure, unary->unary());

  switch (unary->alt()) {
  case FPXUnary::Dec: break;
  case FPXUnary::Inc: break;
  case FPXUnary::LogNot: break;
  case FPXUnary::Member: break;
  case FPXUnary::Neg: break;
  case FPXUnary::Pos: break;
  }
}

void _FIPraeCompiler::emitFunc(FuncClosure &closure, const FPXFunc *func) {
  emitLoadExpr(closure, func->expr());
  closure.emit(FIOpcode::Pop);
}

FIFunction _FIPraeCompiler::compile(const FPXFunc *func) {
  std::vector<std::int8_t> body;

  FuncClosure closure(body);

  emitFunc(closure, func);

  return FIFunction(body);
}
}
