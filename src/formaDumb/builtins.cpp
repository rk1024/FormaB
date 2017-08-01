#include "builtins.hpp"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "ast.hpp"
#include "interpreter.hpp"

namespace frma {
FAtom _FNumber::kw_exp;

bool _FNumber::tryDispatch(std::vector<std::pair<FAtom, FEntity>> keywords,
                           FEntity *ent) {
  if (!kw_exp) kw_exp = FAtom::intern("exp");

  if (keywords.size() == 1 && keywords.at(0).first == kw_exp &&
      keywords.at(0).second->type() == _FNumber::type) {
    auto num = std::dynamic_pointer_cast<_FNumber>(keywords.at(0).second);

    *ent = std::make_shared<_FNumber>(pow(m_num, num->m_num));
    return true;
  }

  return _FEntity::tryDispatch(keywords, ent);
}

bool _FNumber::tryDispatch(FBinaryOp op, FEntity rhs, FEntity *ent) {
  if (rhs->type() == _FNumber::type) {
    auto nrhs = std::dynamic_pointer_cast<_FNumber>(rhs);

    switch (op) {
    case FBinaryOp::Add:
      *ent = std::make_shared<_FNumber>(m_num + nrhs->m_num);
      return true;
    case FBinaryOp::Div:
      *ent = std::make_shared<_FNumber>(m_num / nrhs->m_num);
      return true;
    case FBinaryOp::Equal:
      *ent = _FBool::get(m_num == nrhs->m_num);
      return true;
    case FBinaryOp::Greater:
      *ent = _FBool::get(m_num > nrhs->m_num);
      return true;
    case FBinaryOp::GreaterEq:
      *ent = _FBool::get(m_num >= nrhs->m_num);
      return true;
    case FBinaryOp::Less: *ent = _FBool::get(m_num < nrhs->m_num); return true;
    case FBinaryOp::LessEq:
      *ent = _FBool::get(m_num <= nrhs->m_num);
      return true;
    case FBinaryOp::Mod:
      *ent = std::make_shared<_FNumber>(fmod(m_num, nrhs->m_num));
      return true;
    case FBinaryOp::Mul:
      *ent = std::make_shared<_FNumber>(m_num * nrhs->m_num);
      return true;
    case FBinaryOp::NotEqual:
      *ent = _FBool::get(m_num != nrhs->m_num);
      return true;
    case FBinaryOp::Sub:
      *ent = std::make_shared<_FNumber>(m_num - nrhs->m_num);
      return true;
    default: break;
    }
  }

  return _FEntity::tryDispatch(op, rhs, ent);
}

bool _FNumber::tryRDispatch(FBinaryOp op, FEntity lhs, FEntity *ent) {
  if (lhs->type() == _FNumber::type) {
    auto nlhs = std::dynamic_pointer_cast<_FNumber>(lhs);

    switch (op) {
    case FBinaryOp::Add:
      *ent = std::make_shared<_FNumber>(nlhs->m_num + m_num);
      return true;
    case FBinaryOp::Div:
      *ent = std::make_shared<_FNumber>(nlhs->m_num / m_num);
      return true;
    // case FBinaryOp::Equal:
    //   *ent = std::make_shared<_FNumber>(nlhs->m_num == m_num);
    //   return true;
    // case FBinaryOp::Greater:
    //   *ent = std::make_shared<_FNumber>(nlhs->m_num > m_num);
    //   return true;
    // case FBinaryOp::GreaterEq:
    //   *ent = std::make_shared<_FNumber>(nlhs->m_num >= m_num);
    //   return true;
    // case FBinaryOp::Less:
    //   *ent = std::make_shared<_FNumber>(nlhs->m_num < m_num);
    //   return true;
    // case FBinaryOp::LessEq:
    //   *ent = std::make_shared<_FNumber>(nlhs->m_num <= m_num);
    //   return true;
    case FBinaryOp::Mod:
      *ent = std::make_shared<_FNumber>(fmod(nlhs->m_num, m_num));
      return true;
    case FBinaryOp::Mul:
      *ent = std::make_shared<_FNumber>(nlhs->m_num * m_num);
      return true;
    // case FBinaryOp::NotEqual:
    //   *ent = std::make_shared<_FNumber>(nlhs->m_num + m_num);
    //   return true;
    case FBinaryOp::Sub:
      *ent = std::make_shared<_FNumber>(nlhs->m_num - m_num);
      return true;
    default: break;
    }
  }
  return _FEntity::tryRDispatch(op, lhs, ent);
}

bool _FNumber::tryDispatch(FUnaryOp op, FEntity *ent) {
  switch (op) {
  case FUnaryOp::Neg: *ent = std::make_shared<_FNumber>(-m_num); return true;
  case FUnaryOp::Pos: *ent = std::make_shared<_FNumber>(+m_num); return true;
  default: break;
  }

  return _FEntity::tryDispatch(op, ent);
}

_FNumber::_FNumber(double num) : _FEntity(type), m_num(num) {}

std::string _FNumber::toString() const { return std::to_string(m_num); }



FBool _FBool::s_true, _FBool::s_false;

_FBool::_FBool(bool b) : _FEntity(type), m_bool(b) {}

const FBool _FBool::True() {
  if (!s_true) s_true = FBool(new _FBool(true));

  return s_true;
}

const FBool _FBool::False() {
  if (!s_false) s_false = FBool(new _FBool(false));

  return s_false;
}

const FBool _FBool::get(bool b) { return b ? True() : False(); }

std::string _FBool::toString() const { return m_bool ? "True" : "False"; }

bool _FBool::toBool() const { return m_bool; }



FAtom _FString::kw_replace, _FString::kw_with, _FString::kw_toUpper,
    _FString::kw_toLower;

bool _FString::tryDispatch(FAtom keyword, FEntity *ent) {
  if (!kw_toUpper) kw_toUpper = FAtom::intern("toUpper");
  if (!kw_toLower) kw_toLower = FAtom::intern("toLower");

  if (keyword == kw_toUpper) {
    std::string outString = m_str;
    std::transform(
        outString.begin(), outString.end(), outString.begin(), ::toupper);
    *ent = std::make_shared<_FString>(outString);
    return true;
  }

  if (keyword == kw_toLower) {
    std::string outString = m_str;
    std::transform(
        outString.begin(), outString.end(), outString.begin(), ::tolower);
    *ent = std::make_shared<_FString>(outString);
    return true;
  }

  return _FEntity::tryDispatch(keyword, ent);
}

bool _FString::tryDispatch(std::vector<std::pair<FAtom, FEntity>> keywords,
                           FEntity *ent) {
  if (!kw_replace) kw_replace = FAtom::intern("replace");
  if (!kw_with) kw_with       = FAtom::intern("with");

  if (keywords.size() == 2 && keywords.at(0).first == kw_replace &&
      keywords.at(1).first == kw_with) {
    std::string replace = keywords.at(0).second->toString(),
                with = keywords.at(1).second->toString(), out_str = m_str;

    size_t pos = 0;

    while (true) {
      pos = out_str.find(replace, pos);

      if (pos == std::string::npos) break;

      out_str.replace(pos, replace.size(), with);

      pos += with.size();
    }

    *ent = std::make_shared<_FString>(out_str);
    return true;
  }

  return _FEntity::tryDispatch(keywords, ent);
}

bool _FString::tryDispatch(FBinaryOp op, FEntity rhs, FEntity *ent) {
  if (op == FBinaryOp::Add) {
    *ent = std::make_shared<_FString>(m_str + rhs->toString());
    return true;
  }

  return _FEntity::tryDispatch(op, rhs, ent);
}

bool _FString::tryRDispatch(FBinaryOp op, FEntity lhs, FEntity *ent) {
  if (op == FBinaryOp::Add) {
    *ent = std::make_shared<_FString>(lhs->toString() + m_str);
    return true;
  }

  return _FEntity::tryDispatch(op, lhs, ent);
}

_FString::_FString(const std::string &str) : _FEntity(type), m_str(str) {}

std::string _FString::toString() const { return m_str; }



FAtom _FFunction::kw_call;

bool _FFunction::tryDispatch(FAtom keyword, FEntity *ent) {
  if (!kw_call) kw_call = FAtom::intern("call");

  if (m_params.empty() && keyword == kw_call) {
    *ent = m_interp->run(m_expr, std::make_shared<_FClosure>(m_closure.lock()));
    return true;
  }

  return _FEntity::tryDispatch(keyword, ent);
}

bool _FFunction::tryDispatch(std::vector<std::pair<FAtom, FEntity>> keywords,
                             FEntity *ent) {
  if (m_params.size() == keywords.size()) {
    FClosure closure = std::make_shared<_FClosure>(m_closure.lock());

    for (size_t i = 0; i < m_params.size(); ++i) {
      if (m_params.at(i) != keywords.at(i).first) goto brk;
      closure->bind(keywords.at(i).first, keywords.at(i).second, false);
    }

    *ent = m_interp->run(m_expr, closure);
    return true;
  }

brk:
  return _FEntity::tryDispatch(keywords, ent);
}

void _FFunction::getParams(const FMFuncParams *params) {
  switch (params->alt()) {
  case FMFuncParams::List: getParams(params->params()); break;
  case FMFuncParams::Empty: break;
  case FMFuncParams::Parameters: getParams(params->params());
  case FMFuncParams::Parameter: getParams(params->param()); break;
  }
}

void _FFunction::getParams(const FMFuncParam *param) {
  m_params.push_back(FAtom::intern(
      std::string(param->id()->value(), 0, param->id()->value().size() - 1)));
}

_FFunction::_FFunction(FDumbInterpreter *interp,
                       const FMXFunc *   func,
                       FClosure          closure)
    : _FEntity(type),
      m_interp(interp),
      m_expr(func->expr()),
      m_closure(closure) {
  getParams(func->params());
}



FCout _FCout::s_inst;
FAtom _FCout::kw_print, _FCout::kw_test;

_FCout::_FCout() : _FEntity(type) {}

bool _FCout::tryDispatch(std::vector<std::pair<FAtom, FEntity>> keywords,
                         FEntity *ent) {
  if (!kw_print) kw_print = FAtom::intern("print");

  if (keywords.size() == 1 && keywords.at(0).first == kw_print) {
    std::cout << keywords.at(0).second->toString() << std::endl;
    *ent = s_inst;
    return true;
  }

  return _FEntity::tryDispatch(keywords, ent);
}

const FCout _FCout::instance() {
  if (!s_inst) s_inst = FCout(new _FCout());

  return s_inst;
}
}
