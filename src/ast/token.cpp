#include "ast/token.hpp"

namespace frma {
FormaToken::FormaToken(const std::string &value) : m_value(value) {}

FormaToken::FormaToken(const char *value) : FormaToken(std::string(value)) {}

void FormaToken::print(std::ostream &os) const { os << m_value; }

const std::string &FormaToken::value() const { return m_value; }
}
