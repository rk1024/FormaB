#include <cstdio>

#include "bison-test.hpp"
#include "lexerDriver.hpp"
#include "parserTag.hpp"

int main(int argc, char **argv) {
  FILE *      infile;
  std::string filename("???");

  switch (argc) {
  case 1:
    infile   = stdin;
    filename = "<stdin>";
    break;
  case 2:
    infile   = fopen(argv[1], "r");
    filename = argv[1];
    break;
  default:
    std::cerr << "Usage: " << argv[0] << " [input]" << std::endl;
    return 1;
  }

  try {
    frma::FormaParserTag tag(filename);
    frma::lexer          lex(tag);
    frma::parser         parse(&tag);

    lex.inFile(infile);

    parse.parse();

    // bool fail = !tag.errors().empty();

    for (auto r : tag.errors()) r.print(std::cerr);

    if (tag.prims) tag.prims->print(std::cout);

    std::cout << std::endl;

    std::cerr << tag.errors().size() << " error(s)." << std::endl;

  } catch (std::exception &e) { std::cerr << e.what() << std::endl; }
}
