#include "interpreter.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

namespace frma {
FEntity FDumbInterpreter::run(const FPrims *prims, FClosure closure) {
  switch (prims->alt()) {
  case FPrims::Empty: return nullptr;
  case FPrims::Primaries: run(prims->prims(), closure);
  case FPrims::Primary: return run(prims->prim(), closure);
  }
}

FEntity FDumbInterpreter::run(const FPrim *prim, FClosure) {
  switch (prim->alt()) {
  case FPrim::DQLiteral:
    return std::make_shared<_FString>(
        std::string(prim->tok()->value(), 1, prim->tok()->value().size() - 2));
  case FPrim::Group: break;
  case FPrim::Identifier: break;
  case FPrim::MetaBlock: {
    FClosure _closure = std::make_shared<_FClosure>();
    _closure->bind(FAtom::intern("cout"), _FCout::instance(), false);
    run(prim->metablk(), _closure);
    break;
  }
  case FPrim::Number:
    return std::make_shared<_FNumber>(std::stod(prim->tok()->value()));
  case FPrim::Operator: break;
  case FPrim::PPDirective: break;
  case FPrim::RawBlock: break;
  case FPrim::SQLiteral: break;
  }

  return nullptr;
}

FEntity FDumbInterpreter::run(const FMBlock *blk, FClosure closure) {
  return run(blk->expr(), closure);
}

FEntity FDumbInterpreter::run(const FMStmts *stmts, FClosure closure) {
  switch (stmts->alt()) {
  case FMStmts::Empty: return nullptr;
  case FMStmts::Statements: run(stmts->stmts(), closure);
  case FMStmts::Statement: return run(stmts->stmt(), closure);
  }
}

FEntity FDumbInterpreter::run(const FMStmt *stmt, FClosure closure) {
  switch (stmt->alt()) {
  case FMStmt::Control: return run(stmt->ctl(), closure);
  case FMStmt::Bind: run(stmt->bind(), closure); return nullptr;
  case FMStmt::Assign: return run(stmt->assign(), closure);
  case FMStmt::NonSemiExpr:
  case FMStmt::SemiExpr: return run(stmt->expr(), closure);
  }
}

void FDumbInterpreter::run(const FMSBind *bind, FClosure closure) {
  switch (bind->alt()) {
  case FMSBind::Let: run(bind->binds(), closure, false); break;
  case FMSBind::Var: run(bind->binds(), closure, true); break;
  }
}

void FDumbInterpreter::run(const FMBindings *binds,
                           FClosure          closure,
                           bool              mut) {
  switch (binds->alt()) {
  case FMBindings::Bindings: run(binds->binds(), closure, mut);
  case FMBindings::Binding: run(binds->bind(), closure, mut); break;
  }
}

void FDumbInterpreter::run(const FMBinding *bind, FClosure closure, bool mut) {
  closure->bind(
      FAtom::intern(bind->id()->value()), run(bind->expr(), closure), mut);
}

FEntity FDumbInterpreter::run(const FMExpr *expr, FClosure closure) {
  switch (expr->alt()) {
  case FMExpr::Control: return run(expr->ctl(), closure);
  case FMExpr::Function:
    return std::make_shared<_FFunction>(this, expr->func(), closure);
  case FMExpr::Infix: return run(expr->infix(), closure);
  }
}

FEntity FDumbInterpreter::run(const FMSAssign *assign, FClosure closure) {
  FEntity ent = run(assign->value(), closure);

  switch (assign->alt()) {
  case FMSAssign::Add:
    ent = _FEntity::dispatch(FBinaryOp::Add, run(assign->memb(), closure), ent);
    break;
  case FMSAssign::Assign: break;
  case FMSAssign::Div:
    ent = _FEntity::dispatch(FBinaryOp::Div, run(assign->memb(), closure), ent);
    break;
  case FMSAssign::LogAnd:
    ent = _FEntity::dispatch(
        FBinaryOp::Conjunct, run(assign->memb(), closure), ent);
    break;
  case FMSAssign::LogOr:
    ent = _FEntity::dispatch(
        FBinaryOp::Disjunct, run(assign->memb(), closure), ent);
    break;
  case FMSAssign::Mod:
    ent = _FEntity::dispatch(FBinaryOp::Mod, run(assign->memb(), closure), ent);
    break;
  case FMSAssign::Mul:
    ent = _FEntity::dispatch(FBinaryOp::Mul, run(assign->memb(), closure), ent);
    break;
  case FMSAssign::Sub:
    ent = _FEntity::dispatch(FBinaryOp::Sub, run(assign->memb(), closure), ent);
    break;
  }

  assignLval(assign->memb(), ent, closure);
  return ent;
}

FEntity FDumbInterpreter::run(const FMAssignValue *value, FClosure closure) {
  switch (value->alt()) {
  case FMAssignValue::Assign: return run(value->assign(), closure);
  case FMAssignValue::Infix: return run(value->infix(), closure);
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FMXControl *ctl, FClosure closure) {
  FClosure _closure = std::make_shared<_FClosure>(closure);

  assert(ctl->cond()->alt() == FMXParen::Paren);

  auto cond = ctl->cond()->paren();

  switch (ctl->alt()) {
  case FMXControl::If:
    return run(cond, _closure)->toBool() ? run(ctl->then(), _closure) : nullptr;
  case FMXControl::IfElse:
    return run(cond, _closure)->toBool() ? run(ctl->then(), _closure) :
                                           run(ctl->otherwise(), _closure);
  case FMXControl::IfExpr:
    return run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                           nullptr;
  case FMXControl::IfElseExpr:
    return run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                           run(ctl->otherwiseExpr(), _closure);
  case FMXControl::Unless:
    return !run(cond, _closure)->toBool() ? run(ctl->then(), _closure) :
                                            nullptr;
  case FMXControl::UnlessElse:
    return !run(cond, _closure)->toBool() ? run(ctl->then(), _closure) :
                                            run(ctl->otherwise(), _closure);
  case FMXControl::UnlessExpr:
    return !run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                            nullptr;
  case FMXControl::UnlessElseExpr:
    return !run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                            run(ctl->otherwiseExpr(), _closure);
  case FMXControl::While: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (run(cond->expr(), _closure)->toBool())
      ent = run(ctl->then(), _closure);

    return ent;
  }
  case FMXControl::WhileElse: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (true) {
      if (!run(cond->expr(), _closure)->toBool()) {
        ent = run(ctl->otherwise(), _closure);
        break;
      }

      ent = run(ctl->then(), _closure);
    }

    return ent;
  }
  case FMXControl::WhileExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (run(cond->expr(), _closure)->toBool())
      ent = run(ctl->thenExpr(), _closure);

    return ent;
  }
  case FMXControl::WhileElseExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (true) {
      if (!run(cond->expr(), _closure)->toBool()) {
        ent = run(ctl->otherwiseExpr(), _closure);
        break;
      }

      ent = run(ctl->thenExpr(), _closure);
    }

    return ent;
  }
  case FMXControl::Until: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (!run(cond->expr(), _closure)->toBool())
      ent = run(ctl->then(), _closure);

    return ent;
  }
  case FMXControl::UntilElse: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (true) {
      if (run(cond->expr(), _closure)->toBool()) {
        ent = run(ctl->otherwise(), _closure);
        break;
      }

      ent = run(ctl->then(), _closure);
    }

    return ent;
  }
  case FMXControl::UntilExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (!run(cond->expr(), _closure)->toBool())
      ent = run(ctl->thenExpr(), _closure);

    return ent;
  }
  case FMXControl::UntilElseExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FMXParen::Where);

    run(cond->bind(), _closure);

    while (true) {
      if (run(cond->expr(), _closure)->toBool()) {
        ent = run(ctl->otherwiseExpr(), _closure);
        break;
      }

      ent = run(ctl->thenExpr(), _closure);
    }

    return ent;
  }
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FMXInfix *infix, FClosure closure) {
  if (infix->alt() == FMXInfix::Unary) return run(infix->unary(), closure);

  FEntity lhs = run(infix->infixl(), closure),
          rhs = infix->alt() == FMXInfix::Mod ? run(infix->unary(), closure) :
                                                run(infix->infixr(), closure);

  switch (infix->alt()) {
  case FMXInfix::Add: return _FEntity::dispatch(FBinaryOp::Add, lhs, rhs);
  case FMXInfix::Conjunct:
    return _FEntity::dispatch(FBinaryOp::Conjunct, lhs, rhs);
  case FMXInfix::Disjunct:
    return _FEntity::dispatch(FBinaryOp::Disjunct, lhs, rhs);
  case FMXInfix::Div: return _FEntity::dispatch(FBinaryOp::Div, lhs, rhs);
  case FMXInfix::Equal: return _FEntity::dispatch(FBinaryOp::Equal, lhs, rhs);
  case FMXInfix::Greater:
    return _FEntity::dispatch(FBinaryOp::Greater, lhs, rhs);
  case FMXInfix::GreaterEq:
    return _FEntity::dispatch(FBinaryOp::GreaterEq, lhs, rhs);
  case FMXInfix::Less: return _FEntity::dispatch(FBinaryOp::Less, lhs, rhs);
  case FMXInfix::LessEq: return _FEntity::dispatch(FBinaryOp::LessEq, lhs, rhs);
  case FMXInfix::Mod: return _FEntity::dispatch(FBinaryOp::Mod, lhs, rhs);
  case FMXInfix::Mul: return _FEntity::dispatch(FBinaryOp::Mul, lhs, rhs);
  case FMXInfix::NotEqual:
    return _FEntity::dispatch(FBinaryOp::NotEqual, lhs, rhs);
  case FMXInfix::Sub: return _FEntity::dispatch(FBinaryOp::Sub, lhs, rhs);
  default: break;
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FMXUnary *unary, FClosure closure) {
  if (unary->alt() == FMXUnary::Member) return run(unary->memb(), closure);

  FEntity ent = run(unary->unary(), closure);

  switch (unary->alt()) {
  case FMXUnary::Dec:
    ent =
        _FEntity::dispatch(FBinaryOp::Sub, ent, std::make_shared<_FNumber>(1));
    switch (unary->unary()->alt()) {
    case FMXUnary::Member:
      assignLval(unary->unary()->memb(), ent, closure);
      break;
    default: {
      std::ostringstream oss;
      unary->unary()->print(oss);
      throw std::runtime_error("'" + oss.str() + "': Bad l-value expression.");
    }
    }
    return ent;
  case FMXUnary::Inc:
    ent =
        _FEntity::dispatch(FBinaryOp::Add, ent, std::make_shared<_FNumber>(1));
    switch (unary->unary()->alt()) {
    case FMXUnary::Member:
      assignLval(unary->unary()->memb(), ent, closure);
      break;
    default: {
      std::ostringstream oss;
      unary->unary()->print(oss);
      throw std::runtime_error("'" + oss.str() + "': Bad l-value expression.");
    }
    }
    return ent;
  case FMXUnary::LogNot: return ent->dispatch(FUnaryOp::LogNot);
  case FMXUnary::Neg: return ent->dispatch(FUnaryOp::Neg);
  case FMXUnary::Pos: return ent->dispatch(FUnaryOp::Pos);
  default: break;
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FMXMember *memb, FClosure closure) {
  switch (memb->alt()) {
  case FMXMember::Member:
    break; // TODO
  case FMXMember::Primary: return run(memb->prim(), closure);
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FMXPrim *prim, FClosure closure) {
  switch (prim->alt()) {
  case FMXPrim::Block: return run(prim->block()->stmts(), closure);
  case FMXPrim::Boolean: return getBoolean(prim->boolean());
  case FMXPrim::DQLiteral:
    return std::make_shared<_FString>(
        std::string(prim->tok()->value(), 1, prim->tok()->value().size() - 2));
  case FMXPrim::Identifier:
    return closure->get(FAtom::intern(prim->tok()->value()));
  case FMXPrim::Message: return run(prim->message(), closure);
  case FMXPrim::Number:
    return std::make_shared<_FNumber>(std::stod(prim->tok()->value()));
  case FMXPrim::Parens: return run(prim->paren(), closure);
  case FMXPrim::SQLiteral: break;
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FMXMsg *msg, FClosure closure) {
  std::vector<const FMMsgSelector *> selectors;
  getSelectors(msg->sels(), selectors);

  FEntity ent = run(msg->expr(), closure);

  for (auto sel : selectors) {
    switch (sel->alt()) {
    case FMMsgSelector::Keyword: {
      std::vector<std::pair<FAtom, FEntity>> keywords;
      getKeywords(sel->kws(), keywords, closure);

      if (!ent)
        throw std::runtime_error(keywords.at(0).first.toString() +
                                 ": Attempt to pass message to void");

      ent = ent->dispatch(keywords);
      break;
    }
    case FMMsgSelector::Unary:
      ent = ent->dispatch(FAtom::intern(std::string(sel->tok()->value())));
      break;
    }
  }

  // std::cout << "  \x1b[1m[DEBUG]\x1b[0m Result: ";
  // if (ent)
  //   std::cout << "\x1b[38;5;4m" << ent->toString();
  // else
  //   std::cout << "\x1b[38;5;9m(void)";
  // std::cout << "\x1b[0m" << std::endl;

  return ent;
}

FEntity FDumbInterpreter::run(const FMXParen *paren, FClosure closure) {
  switch (paren->alt()) {
  case FMXParen::Paren: return run(paren->paren(), closure);
  case FMXParen::Tuple:
    switch (paren->exprs()->alt()) {
    case FMExprs::Empty: break;
    case FMExprs::Exprs: break;
    case FMExprs::Expr: return run(paren->exprs()->expr(), closure);
    }
    break;
  case FMXParen::Where: {
    FClosure _closure = std::make_shared<_FClosure>(closure);
    run(paren->bind(), _closure);
    return run(paren->expr(), _closure);
  }
  }

  assert(false);
}

FEntity FDumbInterpreter::runScopedWhere(const FMXParen *paren,
                                         FClosure        closure) {
  assert(paren->alt() == FMXParen::Where);

  run(paren->bind(), closure);
  return run(paren->expr(), closure);
}

void FDumbInterpreter::getSelectors(
    const FMMsgSelectors *sels, std::vector<const FMMsgSelector *> &selectors) {
  switch (sels->alt()) {
  case FMMsgSelectors::Empty: return;
  case FMMsgSelectors::Selectors: getSelectors(sels->sels(), selectors);
  case FMMsgSelectors::Selector: selectors.push_back(sels->sel()); break;
  }
}

void FDumbInterpreter::getKeywords(
    const FMMsgKeywords *kws,
    std::vector<std::pair<FAtom, FEntity>> &keywords,
    FClosure closure) {
  switch (kws->alt()) {
  case FMMsgKeywords::Keywords: getKeywords(kws->kws(), keywords, closure);
  case FMMsgKeywords::Keyword: {
    auto name = kws->kw()->id()->value();
    keywords.push_back(
        std::make_pair(FAtom::intern(std::string(name, 0, name.size() - 1)),
                       run(kws->kw()->expr(), closure)));
    break;
  }
  }
}

FBool FDumbInterpreter::getBoolean(const FMXBoolean *boolean) const {
  switch (boolean->alt()) {
  case FMXBoolean::True: return _FBool::True();
  case FMXBoolean::False: return _FBool::False();
  }

  assert(false);
}

void FDumbInterpreter::assignLval(const FMXMember *memb,
                                  FEntity          ent,
                                  FClosure         closure) {
  switch (memb->alt()) {
  case FMXMember::Member:
    assert(false); // TODO
  case FMXMember::Primary: {
    auto prim = memb->prim();
    switch (prim->alt()) {
    case FMXPrim::Identifier:
      closure->set(FAtom::intern(prim->tok()->value()), ent);
      return;
    default: {
      std::ostringstream oss;
      prim->print(oss);
      throw std::runtime_error("'" + oss.str() + "': Bad l-value expression.");
    }
    }
  }
  }

  assert(false);
}

FDumbInterpreter::FDumbInterpreter(const FPrims *prims) : m_prims(prims) {
  assert(m_prims);
}

void FDumbInterpreter::run() { run(m_prims, std::make_shared<_FClosure>()); }
}
