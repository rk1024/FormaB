#include "bison-test.hpp"
#include "lexerDriver.hpp"
#include "lexerTag.hpp"

int main() {
  void *       scan = nullptr;
  FrmaLexerTag tag;
  int          err = 0;

  if ((err = frma::lex_init_extra(&tag, &scan))) goto catch1;

  {
    frma::parser p(scan);

    p.parse();
  }

  if (tag.error) {
    std::cerr << "Lexical error occurred!" << std::endl;
    err = 1;
    goto catch1;
  }

  goto finally1;

catch1:
  std::cerr << "Something bad happened." << std::endl;

finally1 : {
  int err2 = 0;

  if ((err2 = frma::lex_destroy(scan)))
    std::cerr << "Something *REALLY* bad happened." << std::endl;

  err = err || err2;
}

  return err;
}
