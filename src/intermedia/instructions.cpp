/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (instructions.cpp)
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

#include "instructions.hpp"

namespace fie {
FIOpcode FIBasicInstruction::opcode() const { return m_opcode; }

FIOpcode FILoadInstructionBase::opcode() const { return FIOpcode::Load; }

FIOpcode FIMessageInstruction::opcode() const { return FIOpcode::Msg; }

FIOpcode FITupleInstruction::opcode() const { return FIOpcode::Tpl; }
} // namespace fie
