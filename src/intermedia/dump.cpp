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
void FIDump::writeBlock(const FIBlock *blk) const {
  os() << blk->name() << ":\n";

  for (auto &ins : blk->body()) {
    os() << "  "; // TODO: Add proper indentation

    switch (ins.type()) {
    case FIInstruction::ERR: os() << "ERR"; break;
    case FIInstruction::Drop:
      os() << "drop ";
      writeValue(ins.value());
      break;

    case FIInstruction::Store:
      writeReg(ins.reg());
      os() << " <- ";
      writeValue(ins.value());
    }

    os() << "\n";
  }

  os() << "  ";

  switch (blk->cont()) {
  case FIBlock::ERR: os() << "cont = ERR"; break;
  case FIBlock::Static: os() << "br " << blk->next()->name(); break;
  case FIBlock::Branch:
    os() << "btf ";
    writeReg(blk->reg());
    os() << " " << blk->next()->name() << " " << blk->Else()->name();
    break;
  case FIBlock::Return:
    os() << "ret ";
    writeReg(blk->reg());
    break;
  }

  os() << "\n";
}

void FIDump::writeConst(const FIConst *Const) const {
  os() << "const " << Const->name() << " {\n";
  writeFuncBody(&Const->body());
  os() << "}";
}

void FIDump::writeFoldedConst(const FIFoldedConst *Const) const {
  os() << "const " << Const->name() << " = ";
  writeValue(Const->value());
  os() << ";";
}

void FIDump::writeFuncBody(const FIFunctionBody *body) const {
  for (auto &block : body->blocks()) { writeBlock(block); }
}

void FIDump::writeReg(const FIRegId &reg) const {
  os() << reg.id() << "(" << reg.name() << ")";
}

void FIDump::writeValue(const FIValue *val) const {
  switch (val->type()) {
  case FIValue::Const: {
    auto Const = dynamic_cast<const FIConstValueBase *>(val);

    os() << "const ";

    switch (Const->constType()) {
    case FIConstValueBase::Bool:
      os() << "bool "
           << (dynamic_cast<const FIBoolConstValue *>(Const)->value()
                   ? "true"
                   : "false");
      break;
    case FIConstValueBase::Double:
      os() << "double "
           << dynamic_cast<const FIDoubleConstValue *>(Const)->value();
      break;
    }

    break;
  }
  case FIValue::Msg: {
    auto msg = dynamic_cast<const FIMsgValue *>(val);

    os() << "msg " << msg->msg();

    for (auto &param : msg->params()) {
      os() << " ";
      writeReg(param);
    }

    break;
  }
  case FIValue::Phi: {
    auto phi = dynamic_cast<const FIPhiValue *>(val);

    os() << "phi";

    for (auto &value : phi->values()) {
      os() << " ";
      writeReg(value);
    }

    break;
  }
  }
}

void FIDump::dumpConst(FIConst *Const) {
  writeConst(Const);
  os() << "\n" << std::flush;
}

void FIDump::dumpFoldedConst(FIFoldedConst *Const) {
  writeFoldedConst(Const);
  os() << "\n" << std::flush;
}
} // namespace fie
