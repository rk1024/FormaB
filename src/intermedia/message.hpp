#pragma once

#include <string>
#include <vector>

#include "util/cons.hpp"

namespace fie {
using FIMessageId = fun::cons_cell<bool, std::string>;
}
