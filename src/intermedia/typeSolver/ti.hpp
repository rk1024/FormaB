/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (ti.hpp)
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

#include "algow/ti.hpp"
#include "algow/type.hpp"

#include "intermedia/function/values.hpp"
#include "intermedia/inputs.hpp"
#include "intermedia/typeSolver/typeSolver.hpp"

namespace fie {
class TIContext {
  std::unordered_map<const FIValue *, fun::FPtr<const w::TypeBase>> types;

public:
  const fun::FPtr<const FIInputs>                              inputs;
  const fun::FPtr<const FITypeSolver>                          solver;
  const std::unordered_map<FIRegId, fun::FPtr<const FIValue>> *values;
  w::TypeEnv<FIVariableAtom>                                   env;
  w::Subst                                                     subst;
  w::Constraints                                               constraints;

  constexpr auto &solverFuncs() const { return solver->m_funcs; }
  constexpr auto &solverMsgs() const { return solver->m_msgs; }
  constexpr auto &solverAccepts() const { return solver->m_accepts; }

  auto getType(const FIRegId &reg) const {
    return w::sub(subst, types.at(values->at(reg).get()));
  }

  void putType(const FIRegId &reg, const fun::FPtr<const w::TypeBase> &tp) {
    auto val = values->at(reg).get();
    assert(types.find(val) == types.end());
    types.emplace(val, tp);
  }

  auto getEnv(const FIVariableAtom &var) const {
    return w::sub(subst, env.at(var));
  }

  auto getFunc(const fun::FPtr<const FIFunction> &func) const {
    return w::sub(subst, solver->m_funcs.at(func));
  }

  auto getMsg(const FIMessage &msg) const {
    return w::sub(subst, solver->m_msgs.at(msg));
  }

  TIContext(decltype(inputs) & _inputs,
            decltype(solver) & _solver,
            decltype(*values) &_values) :
      inputs(_inputs),
      solver(_solver),
      values(&_values) {}
};

using TI = w::TI<TIContext>;

class WAcceptsMessageSet {
  FIMessage m_msg;
  const std::unordered_map<FIMessage,
                           std::unordered_set<fun::FRef<w::TypeBase>>>
      *m_accepts;

public:
  WAcceptsMessageSet(
      const FIMessage &msg,
      const std::unordered_map<FIMessage,
                               std::unordered_set<fun::FRef<w::TypeBase>>>
          &accepts) :
      m_msg(msg),
      m_accepts(&accepts) {}

  bool validate(const fun::FRef<const w::TypeBase> &) const;

  bool includes(const w::TypeVars &) const;

  std::string to_string() const;

  bool operator==(const WAcceptsMessageSet &) const;

  friend struct std::hash<WAcceptsMessageSet>;
  friend struct w::Types<WAcceptsMessageSet>;
};

using WAcceptsMessageConstraint = w::IncludesConstraint<WAcceptsMessageSet>;

class WAcceptsMessage : public fun::FObject {
public:
  using CustomType  = w::CustomType<WAcceptsMessage>;
  using UnifyResult = CustomType::UnifyResult;

private:
  FIMessage m_msg;

public:
  WAcceptsMessage(const FIMessage &msg) : m_msg(msg) {}

  UnifyResult mgu(const CustomType &,
                  const fun::FPtr<const w::TypeBase> &,
                  w::TIBase &) const;

  UnifyResult rmgu(const CustomType &                  self,
                   const fun::FPtr<const w::TypeBase> &rhs,
                   w::TIBase &                         t) const {
    return mgu(self, rhs, t);
  }

  std::string to_string() const;

  bool operator==(const WAcceptsMessage &) const;

  friend struct std::hash<WAcceptsMessage>;
};

using WAcceptsMessageType = w::CustomType<WAcceptsMessage>;

// template <typename T>
// class WAcceptsMessage : public w::TypeBase {


// protected:
//   virtual UnifyResult mguImpl(const fun::FPtr<const TypeBase> &rhs,
//                               w::TIBase &t) const override {
//     abort();
//   }

// public:
//   virtual w::TypeVars ftv() const override { abort(); }

//   virtual fun::FPtr<const TypeBase> sub(const w::Subst &s) const override {
//     abort();
//   }

//   virtual std::string to_string() const override {
//     std::ostringstream oss;

//     abort();

//     return oss.str();
//   }

//   virtual void hashImpl(std::size_t &seed) const override { abort(); }

//   virtual bool operator==(const TypeBase &rhs) const override { abort(); }
// };
} // namespace fie

namespace std {
template <>
struct hash<fie::WAcceptsMessageSet> {
  hash<fie::FIMessage> msgHash;

  size_t operator()(const fie::WAcceptsMessageSet &) const;
};

template <>
struct hash<fie::WAcceptsMessage> {
  hash<fie::FIMessage> msgHash;

  size_t operator()(const fie::WAcceptsMessage &) const;
};
} // namespace std

namespace w {
template <>
struct Types<fie::WAcceptsMessageSet> {
  static TypeVars __ftv(const fie::WAcceptsMessageSet &);

  static fie::WAcceptsMessageSet __sub(const Subst &,
                                       const fie::WAcceptsMessageSet &);
};
} // namespace w
