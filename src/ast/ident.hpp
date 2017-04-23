#pragma once

#include "token.hpp"

namespace frma {
class FormaIdent : public FormaToken {
public:
  FormaIdent(const char *text) : FormaToken(text) {}

  friend class FormaPrim;
};
}
