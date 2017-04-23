#include "prim.hpp"

#include <cassert>

#include "group.hpp"

namespace frma {
FormaPrim::FormaPrim(FormaGroup *group) : m_type(Group), m_group(group) {
  m_group->m_rooted = true;
}

FormaPrim::FormaPrim(PrimType type, const std::string &text)
    : m_type(type), m_text(new std::string(text)) {
  switch (type) {
  case Group:
    throw std::logic_error("Constructor not applicable to Group primitives.");
  default: break;
  }
}

FormaPrim::FormaPrim(PrimType type, const char *text)
    : FormaPrim(type, std::string(text)) {}

FormaPrim::~FormaPrim() {
  switch (m_type) {
  case Group: delete m_group; break;
  default: delete m_text; break;
  }
}

void FormaPrim::print(std::ostream &os) const {
  switch (m_type) {
  case Group: m_group->print(os); break;
  default: os << *m_text; break;
  }
}

FormaGroup *FormaPrim::group() const {
  if (m_type == Group) return m_group;
  return nullptr;
}

const std::string *FormaPrim::text() const {
  switch (m_type) {
    case Group: return nullptr;
    default: return m_text;
  }
}
}
