#pragma once

#include "astBase.hpp"

namespace frma {
class FormaPrim;

class FormaPrims : public FormaAST {
  FormaPrim * m_prim;
  FormaPrims *m_prims = nullptr;

public:
  FormaPrims(FormaPrim *expr);

  FormaPrims(FormaPrims *prims, FormaPrim *expr);

  virtual ~FormaPrims() override;

  virtual void print(std::ostream &os) const override;

  friend class FormaGroup;
};
}
