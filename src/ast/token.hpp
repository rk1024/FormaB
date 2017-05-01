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

  friend class FormaPrim;
  friend class FormaRawBlk;
  friend class FormaMSyntaxExt;
};
}
