#pragma once

#include "astBase.hpp"

namespace frma {
class FormaPrims;

class FormaGroup : public FormaAST {
public:
  enum GroupType {
    PGroup,
    KGroup,
    CGroup,
  };

private:
  GroupType   m_type;
  FormaPrims *m_prims;

public:
  FormaGroup(GroupType type, FormaPrims *prims);

  virtual ~FormaGroup() override;

  virtual void print(std::ostream &os) const override;

  inline GroupType type() const { return m_type; }

  inline FormaPrims *prims() const { return m_prims; }

  friend class FormaPrim;
};
}
