#include "namespace.hpp"

namespace frma {
FNamespace FNamespace::s_global("");

FNamespace::FNamespace(const std::string &name) : m_name(name) {}
}
