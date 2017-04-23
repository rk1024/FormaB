#pragma once

#include "token.hpp"

namespace frma {
class FormaPP : public FormaToken {
public:
  FormaPP(const char *text) : FormaToken(text) {}

  friend class FormaPrim;
};
}
