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
  case FPrim::PraeBlock: {
    FClosure _closure = std::make_shared<_FClosure>();
    _closure->bind(FAtom::intern("cout"), _FCout::instance(), false);
    run(prim->praeblk(), _closure);
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

FEntity FDumbInterpreter::run(const FPBlock *blk, FClosure closure) {
  return run(blk->expr(), closure);
}

FEntity FDumbInterpreter::run(const FPStmts *stmts, FClosure closure) {
  switch (stmts->alt()) {
  case FPStmts::Empty: return nullptr;
  case FPStmts::Statements: run(stmts->stmts(), closure);
  case FPStmts::Statement: return run(stmts->stmt(), closure);
  }
}

FEntity FDumbInterpreter::run(const FPStmt *stmt, FClosure closure) {
  switch (stmt->alt()) {
  case FPStmt::Control: return run(stmt->ctl(), closure);
  case FPStmt::Bind: run(stmt->bind(), closure); return nullptr;
  case FPStmt::Assign: return run(stmt->assign(), closure);
  case FPStmt::NonSemiExpr:
  case FPStmt::SemiExpr: return run(stmt->expr(), closure);
  }
}

void FDumbInterpreter::run(const FPSBind *bind, FClosure closure) {
  switch (bind->alt()) {
  case FPSBind::Let: run(bind->binds(), closure, false); break;
  case FPSBind::Var: run(bind->binds(), closure, true); break;
  }
}

void FDumbInterpreter::run(const FPBindings *binds,
                           FClosure          closure,
                           bool              mut) {
  switch (binds->alt()) {
  case FPBindings::Bindings: run(binds->binds(), closure, mut);
  case FPBindings::Binding: run(binds->bind(), closure, mut); break;
  }
}

void FDumbInterpreter::run(const FPBinding *bind, FClosure closure, bool mut) {
  closure->bind(
      FAtom::intern(bind->id()->value()), run(bind->expr(), closure), mut);
}

FEntity FDumbInterpreter::run(const FPExpr *expr, FClosure closure) {
  switch (expr->alt()) {
  case FPExpr::Control: return run(expr->ctl(), closure);
  case FPExpr::Function:
    return std::make_shared<_FFunction>(this, expr->func(), closure);
  case FPExpr::Infix: return run(expr->infix(), closure);
  }
}

FEntity FDumbInterpreter::run(const FPSAssign *assign, FClosure closure) {
  FEntity ent = run(assign->value(), closure);

  switch (assign->alt()) {
  case FPSAssign::Add:
    ent = _FEntity::dispatch(FBinaryOp::Add, run(assign->memb(), closure), ent);
    break;
  case FPSAssign::Assign: break;
  case FPSAssign::Div:
    ent = _FEntity::dispatch(FBinaryOp::Div, run(assign->memb(), closure), ent);
    break;
  case FPSAssign::LogAnd:
    ent = _FEntity::dispatch(
        FBinaryOp::Conjunct, run(assign->memb(), closure), ent);
    break;
  case FPSAssign::LogOr:
    ent = _FEntity::dispatch(
        FBinaryOp::Disjunct, run(assign->memb(), closure), ent);
    break;
  case FPSAssign::Mod:
    ent = _FEntity::dispatch(FBinaryOp::Mod, run(assign->memb(), closure), ent);
    break;
  case FPSAssign::Mul:
    ent = _FEntity::dispatch(FBinaryOp::Mul, run(assign->memb(), closure), ent);
    break;
  case FPSAssign::Sub:
    ent = _FEntity::dispatch(FBinaryOp::Sub, run(assign->memb(), closure), ent);
    break;
  }

  assignLval(assign->memb(), ent, closure);
  return ent;
}

FEntity FDumbInterpreter::run(const FPAssignValue *value, FClosure closure) {
  switch (value->alt()) {
  case FPAssignValue::Assign: return run(value->assign(), closure);
  case FPAssignValue::Infix: return run(value->infix(), closure);
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FPXControl *ctl, FClosure closure) {
  FClosure _closure = std::make_shared<_FClosure>(closure);

  assert(ctl->cond()->alt() == FPXParen::Paren);

  auto cond = ctl->cond()->paren();

  switch (ctl->alt()) {
  case FPXControl::If:
    return run(cond, _closure)->toBool() ? run(ctl->then(), _closure) : nullptr;
  case FPXControl::IfElse:
    return run(cond, _closure)->toBool() ? run(ctl->then(), _closure) :
                                           run(ctl->otherwise(), _closure);
  case FPXControl::IfExpr:
    return run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                           nullptr;
  case FPXControl::IfElseExpr:
    return run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                           run(ctl->otherwiseExpr(), _closure);
  case FPXControl::Unless:
    return !run(cond, _closure)->toBool() ? run(ctl->then(), _closure) :
                                            nullptr;
  case FPXControl::UnlessElse:
    return !run(cond, _closure)->toBool() ? run(ctl->then(), _closure) :
                                            run(ctl->otherwise(), _closure);
  case FPXControl::UnlessExpr:
    return !run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                            nullptr;
  case FPXControl::UnlessElseExpr:
    return !run(cond, _closure)->toBool() ? run(ctl->thenExpr(), _closure) :
                                            run(ctl->otherwiseExpr(), _closure);
  case FPXControl::While: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

    run(cond->bind(), _closure);

    while (run(cond->expr(), _closure)->toBool())
      ent = run(ctl->then(), _closure);

    return ent;
  }
  case FPXControl::WhileElse: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

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
  case FPXControl::WhileExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

    run(cond->bind(), _closure);

    while (run(cond->expr(), _closure)->toBool())
      ent = run(ctl->thenExpr(), _closure);

    return ent;
  }
  case FPXControl::WhileElseExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

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
  case FPXControl::Until: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

    run(cond->bind(), _closure);

    while (!run(cond->expr(), _closure)->toBool())
      ent = run(ctl->then(), _closure);

    return ent;
  }
  case FPXControl::UntilElse: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

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
  case FPXControl::UntilExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

    run(cond->bind(), _closure);

    while (!run(cond->expr(), _closure)->toBool())
      ent = run(ctl->thenExpr(), _closure);

    return ent;
  }
  case FPXControl::UntilElseExpr: {
    FEntity ent = nullptr;

    assert(cond->alt() == FPXParen::Where);

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

FEntity FDumbInterpreter::run(const FPXInfix *infix, FClosure closure) {
  if (infix->alt() == FPXInfix::Unary) return run(infix->unary(), closure);

  FEntity lhs = run(infix->infixl(), closure),
          rhs = infix->alt() == FPXInfix::Mod ? run(infix->unary(), closure) :
                                                run(infix->infixr(), closure);

  switch (infix->alt()) {
  case FPXInfix::Add: return _FEntity::dispatch(FBinaryOp::Add, lhs, rhs);
  case FPXInfix::Conjunct:
    return _FEntity::dispatch(FBinaryOp::Conjunct, lhs, rhs);
  case FPXInfix::Disjunct:
    return _FEntity::dispatch(FBinaryOp::Disjunct, lhs, rhs);
  case FPXInfix::Div: return _FEntity::dispatch(FBinaryOp::Div, lhs, rhs);
  case FPXInfix::Equal: return _FEntity::dispatch(FBinaryOp::Equal, lhs, rhs);
  case FPXInfix::Greater:
    return _FEntity::dispatch(FBinaryOp::Greater, lhs, rhs);
  case FPXInfix::GreaterEq:
    return _FEntity::dispatch(FBinaryOp::GreaterEq, lhs, rhs);
  case FPXInfix::Less: return _FEntity::dispatch(FBinaryOp::Less, lhs, rhs);
  case FPXInfix::LessEq: return _FEntity::dispatch(FBinaryOp::LessEq, lhs, rhs);
  case FPXInfix::Mod: return _FEntity::dispatch(FBinaryOp::Mod, lhs, rhs);
  case FPXInfix::Mul: return _FEntity::dispatch(FBinaryOp::Mul, lhs, rhs);
  case FPXInfix::NotEqual:
    return _FEntity::dispatch(FBinaryOp::NotEqual, lhs, rhs);
  case FPXInfix::Sub: return _FEntity::dispatch(FBinaryOp::Sub, lhs, rhs);
  default: break;
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FPXUnary *unary, FClosure closure) {
  if (unary->alt() == FPXUnary::Member) return run(unary->memb(), closure);

  FEntity ent = run(unary->unary(), closure);

  switch (unary->alt()) {
  case FPXUnary::Dec:
    ent =
        _FEntity::dispatch(FBinaryOp::Sub, ent, std::make_shared<_FNumber>(1));
    switch (unary->unary()->alt()) {
    case FPXUnary::Member:
      assignLval(unary->unary()->memb(), ent, closure);
      break;
    default: {
      std::ostringstream oss;
      unary->unary()->print(oss);
      throw std::runtime_error("'" + oss.str() + "': Bad l-value expression.");
    }
    }
    return ent;
  case FPXUnary::Inc:
    ent =
        _FEntity::dispatch(FBinaryOp::Add, ent, std::make_shared<_FNumber>(1));
    switch (unary->unary()->alt()) {
    case FPXUnary::Member:
      assignLval(unary->unary()->memb(), ent, closure);
      break;
    default: {
      std::ostringstream oss;
      unary->unary()->print(oss);
      throw std::runtime_error("'" + oss.str() + "': Bad l-value expression.");
    }
    }
    return ent;
  case FPXUnary::LogNot: return ent->dispatch(FUnaryOp::LogNot);
  case FPXUnary::Neg: return ent->dispatch(FUnaryOp::Neg);
  case FPXUnary::Pos: return ent->dispatch(FUnaryOp::Pos);
  default: break;
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FPXMember *memb, FClosure closure) {
  switch (memb->alt()) {
  case FPXMember::Member:
    break; // TODO
  case FPXMember::Primary: return run(memb->prim(), closure);
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FPXPrim *prim, FClosure closure) {
  switch (prim->alt()) {
  case FPXPrim::Block: return run(prim->block()->stmts(), closure);
  case FPXPrim::Boolean: return getBoolean(prim->boolean());
  case FPXPrim::DQLiteral:
    return std::make_shared<_FString>(
        std::string(prim->tok()->value(), 1, prim->tok()->value().size() - 2));
  case FPXPrim::Identifier:
    return closure->get(FAtom::intern(prim->tok()->value()));
  case FPXPrim::Message: return run(prim->message(), closure);
  case FPXPrim::Number:
    return std::make_shared<_FNumber>(std::stod(prim->tok()->value()));
  case FPXPrim::Parens: return run(prim->paren(), closure);
  case FPXPrim::SQLiteral: break;
  }

  assert(false);
}

FEntity FDumbInterpreter::run(const FPXMsg *msg, FClosure closure) {
  std::vector<const FPMsgSelector *> selectors;
  getSelectors(msg->sels(), selectors);

  FEntity ent = run(msg->expr(), closure);

  for (auto sel : selectors) {
    switch (sel->alt()) {
    case FPMsgSelector::Keyword: {
      std::vector<std::pair<FAtom, FEntity>> keywords;
      getKeywords(sel->kws(), keywords, closure);

      if (!ent)
        throw std::runtime_error(keywords.at(0).first.toString() +
                                 ": Attempt to pass message to void");

      ent = ent->dispatch(keywords);
      break;
    }
    case FPMsgSelector::Unary:
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

FEntity FDumbInterpreter::run(const FPXParen *paren, FClosure closure) {
  switch (paren->alt()) {
  case FPXParen::Paren: return run(paren->paren(), closure);
  case FPXParen::Tuple:
    switch (paren->exprs()->alt()) {
    case FPExprs::Empty: break;
    case FPExprs::Exprs: break;
    case FPExprs::Expr: return run(paren->exprs()->expr(), closure);
    }
    break;
  case FPXParen::Where: {
    FClosure _closure = std::make_shared<_FClosure>(closure);
    run(paren->bind(), _closure);
    return run(paren->expr(), _closure);
  }
  }

  assert(false);
}

FEntity FDumbInterpreter::runScopedWhere(const FPXParen *paren,
                                         FClosure        closure) {
  assert(paren->alt() == FPXParen::Where);

  run(paren->bind(), closure);
  return run(paren->expr(), closure);
}

void FDumbInterpreter::getSelectors(
    const FPMsgSelectors *sels, std::vector<const FPMsgSelector *> &selectors) {
  switch (sels->alt()) {
  case FPMsgSelectors::Empty: return;
  case FPMsgSelectors::Selectors: getSelectors(sels->sels(), selectors);
  case FPMsgSelectors::Selector: selectors.push_back(sels->sel()); break;
  }
}

void FDumbInterpreter::getKeywords(
    const FPMsgKeywords *kws,
    std::vector<std::pair<FAtom, FEntity>> &keywords,
    FClosure closure) {
  switch (kws->alt()) {
  case FPMsgKeywords::Keywords: getKeywords(kws->kws(), keywords, closure);
  case FPMsgKeywords::Keyword: {
    auto name = kws->kw()->id()->value();
    keywords.push_back(
        std::make_pair(FAtom::intern(std::string(name, 0, name.size() - 1)),
                       run(kws->kw()->expr(), closure)));
    break;
  }
  }
}

FBool FDumbInterpreter::getBoolean(const FPXBoolean *boolean) const {
  switch (boolean->alt()) {
  case FPXBoolean::True: return _FBool::True();
  case FPXBoolean::False: return _FBool::False();
  }

  assert(false);
}

void FDumbInterpreter::assignLval(const FPXMember *memb,
                                  FEntity          ent,
                                  FClosure         closure) {
  switch (memb->alt()) {
  case FPXMember::Member:
    assert(false); // TODO
  case FPXMember::Primary: {
    auto prim = memb->prim();
    switch (prim->alt()) {
    case FPXPrim::Identifier:
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
