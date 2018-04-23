/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (dump.cpp)
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

#include "dump.hpp"

namespace fie {
void FIDump::writeValue(const FIValue *val) {
  switch (val->type()) {
  case FIValue::Const: {
    auto konst = dynamic_cast<const FIConstBase *>(val);

    switch (konst->constType()) {
    case FIConstBase::Double:
      os() << dynamic_cast<const FIDoubleConst *>(konst)->value();
      break;
    }

    break;
  }
  }
}

void FIDump::dumpGlobalConstant(FIGlobalConstant *konst) {
  os() << konst->name() << " = ";
  writeValue(konst->value());
  os() << ";" << std::endl;
}
} // namespace fie
