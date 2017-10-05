#pragma once

#include "util/consPod.hpp"

namespace fie {
FUN_CONSPOD(FIVariable, std::string) {
  FCP_GET(0, name);

  inline FIVariable(const std::string &_name) : FCP_INIT(_name) {}
};
}

FCP_HASH(fie::FIVariable)
