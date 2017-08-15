#pragma once

#include <iostream>
#include <sstream>
#include <string>

namespace frma {
class FormaAST {
protected:
  bool m_rooted = false;

public:
  virtual ~FormaAST() {}

  virtual void print(std::ostream &) const {}

  inline bool rooted() const { return m_rooted; }

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
