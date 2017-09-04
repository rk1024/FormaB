#include "astBase.hpp"

namespace frma {
FormaAST::FormaAST(const location &loc) : m_loc(loc) {}

FormaAST::~FormaAST() {}

void FormaAST::print(std::ostream &) const {}
}
