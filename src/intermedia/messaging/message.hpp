#pragma once

#include <vector>

#include "util/consPod.hpp"

namespace fie {
FUN_CONSPOD(FIMessage, std::uint32_t, std::string) {
  FCP_GET(0, arity);
  FCP_GET(1, name);

  inline FIMessage(std::uint32_t _arity, std::string _name)
      : FCP_INIT(_arity, _name) {}
};

FUN_CONSPOD(FIMessageKeyword, bool, std::string) {
  FCP_GET(0, arg);
  FCP_GET(1, name);

  inline FIMessageKeyword(bool _arg, std::string _name)
      : FCP_INIT(_arg, _name) {}
};
}

FCP_HASH(fie::FIMessage)
FCP_HASH(fie::FIMessageKeyword)
