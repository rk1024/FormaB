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
    Identifier,
    PPDirective,
    Literal,
    Operator,
  };

private:
  PrimType m_type;

  union {
    FormaGroup *  m_group;
    FormaIdent *  m_ident;
    FormaPP *     m_pp;
    FormaLiteral *m_literal;
    FormaOper *   m_oper;
  };

public:
  FormaPrim(FormaGroup *group);
  FormaPrim(FormaIdent *ident);
  FormaPrim(FormaPP *pp);
  FormaPrim(FormaLiteral *literal);
  FormaPrim(FormaOper *oper);

  virtual ~FormaPrim() override;

  virtual void print(std::ostream &os) const override;

  FormaGroup *  group() const;
  FormaIdent *  ident() const;
  FormaPP *     pp() const;
  FormaLiteral *literal() const;
  FormaOper *   oper() const;

  friend class FormaPrims;
};
}
