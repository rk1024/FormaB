#pragma once

#include "token.hpp"

namespace frma {
class FormaOper : public FormaToken {
public:
  FormaOper(const char *text) : FormaToken(text) {}

  friend class FormaPrim;
};
}
