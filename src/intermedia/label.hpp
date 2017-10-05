#pragma once

#include "util/consPod.hpp"

namespace fie {
FUN_CONSPOD(FILabel, std::uint32_t, std::string) {
  FCP_ACC(0, pos);
  FCP_ACC(1, name);

  inline FILabel(std::uint32_t _pos, std::string _name)
      : FCP_INIT(_pos, _name) {}
};
}

FCP_HASH(fie::FILabel);
