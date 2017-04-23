#include "prims.hpp"

#include <cassert>

#include "prim.hpp"

namespace frma {
FormaPrims::FormaPrims(FormaPrim *expr) : m_prim(expr) {
  m_prim->m_rooted = true;
}

FormaPrims::FormaPrims(FormaPrims *prims, FormaPrim *expr)
    : m_prim(expr), m_prims(prims) {
  m_prim->m_rooted  = true;
  m_prims->m_rooted = true;
}

FormaPrims::~FormaPrims() {
  delete m_prim;

  if (m_prims) delete m_prims;
}

void FormaPrims::print(std::ostream &os) const {
  if (m_prims) {
    m_prims->print(os);
    os << " ";

    assert(m_prim);
  }

  if (m_prim) m_prim->print(os);
}
}
