#include <cstdio>

#include <list>

#include "lexerDriver.hpp"
#include "parser.hpp"
#include "parserTag.hpp"

using namespace frma;

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

      FPrims &prims = *tag.prims;

      switch (prims.alt()) {
      case FPrims::Empty: break;
      case FPrims::Primaries: break;
      case FPrims::Primary: break;
      }
    }

    std::cout << std::endl;

    std::cerr << tag.errors().size() << " error";
    if (tag.errors().size() != 1) std::cerr << "s";
    std::cerr << "." << std::endl;

    return tag.errors().size() ? 1 : 0;
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
}
