#include <string>

#include "ast/astBase.hpp"

namespace frma {
class FToken : public FormaAST {
  std::string m_value;

public:
  FToken(const std::string &);
  FToken(const char *);

  virtual void print(std::ostream &os) const override;

  const std::string &value() const;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma astgen friends (  )
#pragma clang diagnostic pop
};
}
