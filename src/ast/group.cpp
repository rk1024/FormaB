#include "group.hpp"

#include "prims.hpp"


namespace frma {
FormaGroup::FormaGroup(GroupType type, FormaPrims *prims)
    : m_type(type), m_prims(prims) {
  prims->m_rooted = true;
}

FormaGroup::~FormaGroup() { delete m_prims; }

void FormaGroup::print(std::ostream &os) const {
  switch (m_type) {
  case PGroup: os << "("; break;
  case KGroup: os << "["; break;
  case CGroup: os << "{"; break;
  }

  os << " ";
  m_prims->print(os);
  os << " ";

  switch (m_type) {
  case PGroup: os << ")"; break;
  case KGroup: os << "]"; break;
  case CGroup: os << "}"; break;
  }
}
}
