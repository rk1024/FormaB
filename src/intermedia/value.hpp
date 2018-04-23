/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (value.hpp)
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

#include "diagnostic/location.hpp"

namespace fie {
class FIValue {
public:
  enum Type {
    Const,
  };

private:
  fdi::FLocation m_loc;

public:
  constexpr auto &loc() const { return m_loc; }

  FIValue(const fdi::FLocation &loc) : m_loc(loc) {}

  virtual ~FIValue();

  virtual Type type() const = 0;
};

class FIConstBase : public FIValue {
public:
  enum ConstType {
    Double,
  };

  FIConstBase(const fdi::FLocation &loc) : FIValue(loc) {}

  virtual Type type() const override;

  virtual ConstType constType() const = 0;
};

template <typename>
struct _constTraits;

template <>
struct _constTraits<double> {
  static constexpr FIConstBase::ConstType constType = FIConstBase::Double;
};

template <typename T>
class FIConst : public FIConstBase {
  T m_value;

public:
  constexpr auto &value() const { return m_value; }

  FIConst(const fdi::FLocation &loc, const T &value) :
      FIConstBase(loc),
      m_value(value) {}

  FIConst(const fdi::FLocation &loc, T &&value) :
      FIConstBase(loc),
      m_value(value) {}

  virtual ConstType constType() const override {
    return _constTraits<T>::constType;
  }
};

using FIDoubleConst = FIConst<double>;
} // namespace fie
