#include <string>

#include "ast/astBase.hpp"

namespace frma {
class FormaToken : public FormaAST {
  std::string m_value;

public:
  FormaToken(const std::string &);
  FormaToken(const char *);

  virtual void print(std::ostream &os) const override;

  const std::string &value() const;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma astgen friends
#pragma clang diagnostic pop
};
}
