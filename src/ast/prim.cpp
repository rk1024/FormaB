#include "prim.hpp"

#include <cassert>

#include "group.hpp"
#include "ident.hpp"
#include "literal.hpp"
#include "oper.hpp"
#include "pp.hpp"

namespace frma {
FormaPrim::FormaPrim(FormaGroup *group) : m_type(Group), m_group(group) {
  m_group->m_rooted = true;
}
FormaPrim::FormaPrim(FormaIdent *ident) : m_type(Identifier), m_ident(ident) {
  m_ident->m_rooted = true;
}
FormaPrim::FormaPrim(FormaPP *pp) : m_type(PPDirective), m_pp(pp) {
  m_pp->m_rooted = true;
}
FormaPrim::FormaPrim(FormaLiteral *literal)
    : m_type(Literal), m_literal(literal) {
  m_literal->m_rooted = true;
}
FormaPrim::FormaPrim(FormaOper *oper) : m_type(Operator), m_oper(oper) {
  m_oper->m_rooted = true;
}

FormaPrim::~FormaPrim() {
  switch (m_type) {
  case Group: delete m_group; break;
  case Identifier: delete m_ident; break;
  case PPDirective: delete m_pp; break;
  case Literal: delete m_literal; break;
  case Operator: delete m_oper; break;
  }
}

void FormaPrim::print(std::ostream &os) const {
  switch (m_type) {
  case Group: m_group->print(os); break;
  case Identifier: m_ident->print(os); break;
  case PPDirective: m_pp->print(os); break;
  case Literal: m_literal->print(os); break;
  case Operator: m_oper->print(os); break;
  }
}

FormaLiteral *FormaPrim::literal() const {
  if (m_type == Literal) return m_literal;
  return nullptr;
}

FormaGroup *FormaPrim::group() const {
  if (m_type == Group) return m_group;
  return nullptr;
}
}
