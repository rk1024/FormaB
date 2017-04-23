#include "bison-test.hpp"
#include "lexerDriver.hpp"
#include "parserTag.hpp"

int main() {
  void *       scan = nullptr;
  frma::FormaParserTag tag;
  int          err = 0;

  if ((err = frma::lex_init_extra(&tag, &scan))) goto catch1;

  {
    tag.scan = scan;
    frma::parser parse(&tag);

    parse.parse();

    // bool fail = !tag.errors().empty();

    for (auto r : tag.errors()) r.print(std::cerr);

    if (tag.prims) tag.prims->print(std::cout);

    std::cout << std::endl;

    std::cerr << tag.errors().size() << " error(s)." << std::endl;
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
