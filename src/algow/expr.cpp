/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (expr.cpp)
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

#include "expr.hpp"

#include <sstream>

#include "ti.hpp"
#include "type.hpp"

namespace w {
ExprBase::TIResult ExprBase::ti(const TypeEnv &env, TI &t) const {
  TIPos _(t, "\e[1mti\e[0m " + to_string() + printEnv(env));
  auto  ret = tiImpl(env, t);
  TIPos __(t,
           "\e[38;5;2mresult:\e[0m " + to_string() +
               " :: " + ret.second->to_string() + printSubst(ret.first));
  return ret;
}

ExprBase::TIResult ExprVar::tiImpl(const TypeEnv &env, TI &t) const {
  auto it = env.find(m_var);
  if (it == env.end())
    throw std::runtime_error("unbound variable " + m_var + "\n" + t.state());
  else
    return TIResult(Subst(), t.instantiate(it->second));
}

std::string ExprVar::to_string() const { return m_var; }

ExprBase::TIResult ExprApp::tiImpl(const TypeEnv &env, TI &t) const {
  auto a       = t.makeVar();
  auto[sl, tl] = m_lhs->ti(env, t);
  auto[sr, tr] = m_rhs->ti(w::sub(sl, env), t);
  auto su      = w::sub(sr, tl)->mgu(fnew<TypeToy>(fnew<ToyT>("fun", 2),
                                              TypeToy::Params{{tr, a}}),
                                t);
  return TIResult(composeSubst(composeSubst(su, sr), sl), w::sub(su, a));
}

std::string ExprApp::to_string() const {
  return m_lhs->to_string() + " " + m_rhs->to_string();
}

ExprBase::TIResult ExprAbs::tiImpl(const TypeEnv &env, TI &t) const {
  auto a       = t.makeVar();
  auto env2    = env;
  env2[m_x]    = Scheme(std::vector<std::string>(), a);
  auto[sb, tb] = m_body->ti(env2, t);
  return TIResult(sb,
                  fnew<TypeToy>(fnew<ToyT>("fun", 2),
                                TypeToy::Params{{w::sub(sb, a), tb}}));
}

std::string ExprAbs::to_string() const {
  return "λ" + m_x + "." + m_body->to_string();
}

ExprBase::TIResult ExprLet::tiImpl(const TypeEnv &env, TI &t) const {
  auto[sx, tx] = m_bind->ti(env, t);
  auto tx2     = generalize(w::sub(sx, env), tx);
  auto env2    = env;
  env2[m_x]    = tx2;
  auto[sb, tb] = m_body->ti(w::sub(sx, env2), t);
  return TIResult(composeSubst(sx, sb), tb);
}

std::string ExprLet::to_string() const {
  return "let " + m_x + " = " + m_bind->to_string() + " in " +
         m_body->to_string();
}
} // namespace w
