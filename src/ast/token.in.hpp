#include <string>

#include "ast/astBase.hpp"

#pragma astgen friends forward "  "

namespace frma {
class FToken : public FormaAST {
  std::string m_value;

public:
  FToken(const std::string &, const location &loc);
  FToken(const char *, const location &loc);

  virtual void print(std::ostream &os) const override;

  const std::string &value() const;

#pragma astgen friends "  "
};
}
