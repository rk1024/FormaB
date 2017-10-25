/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (token.cpp)
* Copyright (C) 2017 Ryan Schroeder, Colin Unger
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
*************************************************************************/

#include "ast/token.hpp"

namespace frma {
FToken::FToken(const std::string &value, const location &loc)
    : FormaAST(loc), m_value(value) {}

FToken::FToken(const char *value, const location &loc)
    : FToken(std::string(value), loc) {}

void FToken::print(std::ostream &os) const { os << m_value; }

const std::string &FToken::value() const { return m_value; }
}
