#pragma once

#include <cstdint>

#include "util/atom.hpp"
#include "util/object/object.hpp"
#include "util/ptr.hpp"

#include "function.hpp"

namespace fie {
class FIAssembly : public fun::FObject {
  fun::FAtomStore<fun::FPtr<FIFunction>, std::uint32_t> m_funcs;
};
}
