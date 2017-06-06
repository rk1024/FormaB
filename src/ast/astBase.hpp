#pragma once

#include <iostream>

namespace frma {
class FormaAST {
protected:
  bool m_rooted = false;

public:
  virtual ~FormaAST() {}

  virtual void print(std::ostream &) const {}

  inline bool rooted() const { return m_rooted; }
};
}
