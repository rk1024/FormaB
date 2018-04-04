/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (_expr.hpp)
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

#pragma once

#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "subst.hpp"
#include "typeEnv.hpp"

namespace w {
template <typename>
struct TI;
class TypeBase;

template <typename T>
class ExprBase : public fun::FObject {
public:
  using TIResult =
      fun::cons_cell<Subst, Constraints, fun::FPtr<const TypeBase>>;

protected:
  virtual TIResult tiImpl(TI<T> &) const = 0;

public:
  TIResult ti(TI<T> &) const;

  virtual std::string to_string() const = 0;
};

// class ExprVar : public ExprBase {
//   std::string m_var;

// protected:
//   virtual TIResult tiImpl(const TypeEnv &, TIBase &) const;

// public:
//   constexpr auto &var() const { return m_var; }

//   ExprVar(const std::string &var) : m_var(var) {}

//   virtual std::string to_string() const;
// };

// class ExprApp : public ExprBase {
//   fun::FPtr<const ExprBase> m_lhs, m_rhs;

// protected:
//   virtual TIResult tiImpl(const TypeEnv &, TIBase &) const;

// public:
//   constexpr auto &lhs() const { return m_lhs; }
//   constexpr auto &rhs() const { return m_rhs; }

//   ExprApp(const fun::FPtr<const ExprBase> &lhs,
//           const fun::FPtr<const ExprBase> &rhs) :
//       m_lhs(lhs),
//       m_rhs(rhs) {}

//   virtual std::string to_string() const;
// };

// class ExprAbs : public ExprBase {
//   std::string               m_x;
//   fun::FPtr<const ExprBase> m_body;

// protected:
//   virtual TIResult tiImpl(const TypeEnv &, TIBase &) const;

// public:
//   constexpr auto &x() const { return m_x; }
//   constexpr auto &body() const { return m_body; }

//   ExprAbs(const std::string &x, const fun::FPtr<const ExprBase> &body) :
//       m_x(x),
//       m_body(body) {}

//   virtual std::string to_string() const;
// };

// class ExprLet : public ExprBase {
//   std::string               m_x;
//   fun::FPtr<const ExprBase> m_bind, m_body;

// protected:
//   virtual TIResult tiImpl(const TypeEnv &, TIBase &) const;

// public:
//   constexpr auto &x() const { return m_x; }
//   constexpr auto &bind() const { return m_bind; }
//   constexpr auto &body() const { return m_body; }

//   ExprLet(const std::string &              x,
//           const fun::FPtr<const ExprBase> &bind,
//           const fun::FPtr<const ExprBase> &body) :
//       m_x(x),
//       m_bind(bind),
//       m_body(body) {}

//   virtual std::string to_string() const;
// };

// template <typename T, typename TCtx>
// class Expr : public ExprBase<TCtx> {
//   const T *m_value;

// protected:
//   virtual TIResult tiImpl(const TypeEnv &env, TI<TCtx> &t) const override {
//     return m_value->ti(env, t);
//   }

// public:
//   Expr(const T &value) : m_value(&value) {}

//   virtual std::string to_string() const override { return m_value->toString(); }
// };
} // namespace w
