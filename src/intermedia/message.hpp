#pragma once

#include <string>
#include <vector>

#include "util/cons.hpp"

namespace fie {
using FIMessageId = fun::cons_cell<std::uint32_t, std::string>;

using FIMessageKeyword = fun::cons_cell<bool, std::string>;
}
