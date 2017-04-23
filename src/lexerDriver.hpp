#pragma once

#include "parserTag.hpp"

namespace frma {
  class lexer {
    void *m_yyscanner;

  public:
    lexer(frma::FormaParserTag &tag);

    ~lexer();

    void restart(FILE *input_file);

#ifdef _DEBUG
    bool debug() const;

    void debug(bool value);
#endif

    FILE *inFile() const;

    void inFile(FILE *value);

    FILE *outFile() const;

    void outFile(FILE *value);
  };
}
