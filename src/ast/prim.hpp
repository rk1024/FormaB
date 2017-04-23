#pragma once

#include "astBase.hpp"

namespace frma {
class FormaGroup;
class FormaIdent;
class FormaPP;
class FormaLiteral;
class FormaOper;

class FormaPrim : public FormaAST {
public:
  enum PrimType {
    Group,
    RawBlock,
    Identifier,
    PPDirective,
    Number,
    Operator,
    SQLiteral,
    DQLiteral,
  };

private:
  PrimType m_type;

  union {
    FormaGroup * m_group;
    std::string *m_text;
  };

public:
  FormaPrim(FormaGroup *group);
  FormaPrim(PrimType type, const std::string &text);
  FormaPrim(PrimType type, const char *text);

  virtual ~FormaPrim() override;

  virtual void print(std::ostream &os) const override;

  inline PrimType type() const { return m_type; }

  FormaGroup *       group() const;
  const std::string *text() const;

  friend class FormaPrims;
};
}
