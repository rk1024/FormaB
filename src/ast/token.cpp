#include "ast/token.hpp"

namespace frma {
FToken::FToken(const std::string &value) : m_value(value) {}

FToken::FToken(const char *value) : FToken(std::string(value)) {}

void FToken::print(std::ostream &os) const { os << m_value; }

const std::string &FToken::value() const { return m_value; }
}
