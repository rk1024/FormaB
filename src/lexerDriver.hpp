#pragma once

#include "lexerTag.hpp"

namespace frma {
  int lex_init_extra(FrmaLexerTag *extra, void **scan);

  int lex_destroy(void *scan);
}
