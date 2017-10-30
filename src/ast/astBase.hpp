/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (astBase.hpp)
* Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
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

#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "location.hh"

namespace frma {
class FormaAST {
protected:
  bool     m_rooted = false;
  location m_loc;

public:
  inline bool            rooted() const { return m_rooted; }
  inline const location &loc() const { return m_loc; }

  FormaAST(const location &);

  virtual ~FormaAST();

  virtual void print(std::ostream &) const;

  std::string toString() const {
    std::ostringstream oss;
    print(oss);
    return oss.str();
  }
};

inline std::ostream &operator<<(std::ostream &os, const FormaAST &ast) {
  ast.print(os);
  return os;
}
inline std::ostream &operator<<(std::ostream &os, const FormaAST *ast) {
  ast->print(os);
  return os;
}
}
