#include <cstdio>

#include <list>

#include "bison-test.hpp"
#include "lexerDriver.hpp"
#include "parserTag.hpp"

int main(int argc, char **argv) {
  FILE *      infile;
  std::string filename("???");
#ifdef _DEBUG
  bool verbose = false;
#endif

  std::list<std::string> args;

  {
    bool doFlags = true;
    for (size_t i = 1; i < argc; ++i) {
      std::string arg(argv[i]);
      if (doFlags) {
#ifdef _DEBUG
        if (arg == "-v")
          verbose = true;
        else
#endif
            if (arg == "--")
          doFlags = false;
        else
          args.push_back(arg);
      } else
        args.push_back(arg);
    }
  }
  switch (args.size()) {
  case 0:
    infile   = stdin;
    filename = "<stdin>";
    break;
  case 1:
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

#ifdef _DEBUG
    if (verbose) {
      lex.debug(true);
      parse.set_debug_level(1);
    }
#endif

    bool success = !parse.parse();

    success = success && tag.errors().empty();

    for (auto r : tag.errors()) r.print(std::cerr);

    if (success) {
      if (tag.prims)
        tag.prims->print(std::cout);
      else
        std::cerr << "WARNING: no output" << std::endl;
    }

    std::cout << std::endl;

    std::cerr << tag.errors().size() << " error(s)." << std::endl;

  } catch (std::exception &e) { std::cerr << e.what() << std::endl; }
}
