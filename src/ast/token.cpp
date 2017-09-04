#include "ast/token.hpp"

namespace frma {
FToken::FToken(const std::string &value, const location &loc)
    : FormaAST(loc), m_value(value) {}

FToken::FToken(const char *value, const location &loc)
    : FToken(std::string(value), loc) {}

void FToken::print(std::ostream &os) const { os << m_value; }

const std::string &FToken::value() const { return m_value; }
}
