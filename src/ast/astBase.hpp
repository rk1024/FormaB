#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include "location.hh"

namespace frma {
class FormaAST {
protected:
  bool m_rooted = false;
  location m_loc;

public:
  inline bool            rooted() const { return m_rooted; }
  inline const location &loc() const { return m_loc; }

  FormaAST(const location &loc) : m_loc(loc) {}

  virtual ~FormaAST() {}

  virtual void print(std::ostream &) const {}

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
