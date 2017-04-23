#pragma once

#include "parserTag.hpp"

namespace frma {
  int lex_init_extra(frma::FormaParserTag *extra, void **scan);

  int lex_destroy(void *scan);
}
