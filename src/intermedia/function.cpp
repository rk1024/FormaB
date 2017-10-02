#include "function.hpp"

namespace fie {
FIFunction::FIFunction(std::unordered_map<std::uint32_t, std::uint32_t> args,
                       const FIBytecode body)
    : m_args(args), m_body(body) {}
}
