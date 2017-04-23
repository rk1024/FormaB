#pragma once

#include "astBase.hpp"

namespace frma {
class FormaToken : public FormaAST {
  std::string m_text;

public:
  FormaToken(const char *text);

  virtual void print(std::ostream &os) const override;

  inline const std::string &text() const { return m_text; }
};
}
