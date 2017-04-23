#include "token.hpp"

namespace frma {
FormaToken::FormaToken(const char *text) : m_text(text) {}

void FormaToken::print(std::ostream &os) const { os << m_text; }
}
