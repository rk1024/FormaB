/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (_value.hpp)
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

#include <vector>

#include "diagnostic/location.hpp"

#include "evalContext.hpp"
#include "regId.hpp"

namespace fie {
class FIMessageBase;
class FIContext;
class FIBlock;

class FIValue {
public:
  enum Type {
    Const,
    Msg,
    Phi,
    Var,
  };

private:
  fdi::FLocation m_loc;

public:
  constexpr auto &loc() const { return m_loc; }

  FIValue(const fdi::FLocation &loc) : m_loc(loc) {}

  FIValue(const FIValue &) = default;
  FIValue(FIValue &&)      = default;

  virtual ~FIValue();

  virtual Type type() const = 0;

  virtual FIValue *eval(FIContext &, const FIEvalContext &) const = 0;
};

class FIConstValueBase : public FIValue {
public:
  enum ConstType {
    Bool,
    Double,
  };

  FIConstValueBase(const fdi::FLocation &loc) : FIValue(loc) {}

  virtual Type type() const override;

  virtual ConstType constType() const = 0;
};

template <typename>
struct _constTraits;

template <>
struct _constTraits<bool> {
  static constexpr FIConstValueBase::ConstType
      constType = FIConstValueBase::Bool;
};

template <>
struct _constTraits<double> {
  static constexpr FIConstValueBase::ConstType
      constType = FIConstValueBase::Double;
};

template <typename T>
class FIConstValue : public FIConstValueBase {
  T m_value;

public:
  constexpr auto &value() const { return m_value; }

  FIConstValue(const fdi::FLocation &loc, const T &value) :
      FIConstValueBase(loc),
      m_value(value) {}

  FIConstValue(const fdi::FLocation &loc, T &&value) :
      FIConstValueBase(loc),
      m_value(value) {}

  virtual ConstType constType() const override {
    return _constTraits<T>::constType;
  }

  virtual FIValue *eval(FIContext &, const FIEvalContext &) const override;
};

using FIBoolConstValue   = FIConstValue<bool>;
using FIDoubleConstValue = FIConstValue<double>;

class FIMsgValue : public FIValue {
  FIMessageBase *      m_msg;
  std::vector<FIRegId> m_params;

public:
  constexpr auto &msg() const { return m_msg; }
  constexpr auto &params() const { return m_params; }

  FIMsgValue(const fdi::FLocation &      loc,
             FIMessageBase *             msg,
             const std::vector<FIRegId> &params) :
      FIValue(loc),
      m_msg(msg),
      m_params(params) {}

  virtual Type type() const override;

  virtual FIValue *eval(FIContext &, const FIEvalContext &) const override;
};

class FIPhiValue : public FIValue {
  std::vector<std::pair<FIBlock *, FIRegId>> m_values;

public:
  constexpr auto &values() const { return m_values; }

  FIPhiValue(const fdi::FLocation &                            loc,
             const std::vector<std::pair<FIBlock *, FIRegId>> &values) :
      FIValue(loc),
      m_values(values) {}

  virtual Type type() const override;

  virtual FIValue *eval(FIContext &, const FIEvalContext &) const override;
};

class FIVarValue : public FIValue {
  std::string m_name;

public:
  constexpr auto &name() const { return m_name; }

  FIVarValue(const fdi::FLocation &loc, const std::string &name) :
      FIValue(loc),
      m_name(name) {}

  virtual Type type() const override;

  virtual FIValue *eval(FIContext &, const FIEvalContext &) const override;
};
} // namespace fie
