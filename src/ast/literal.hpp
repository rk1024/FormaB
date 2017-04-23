#pragma once

#include "token.hpp"

namespace frma {
class FormaLiteral : public FormaToken {
public:
  FormaLiteral(const char *text) : FormaToken(text) {}

  friend class FormaPrim;
};
}
