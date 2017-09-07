#include <cstdio>
#include <list>

#include "pipeline/functional.hpp"

#include "intermedia/debug/dumpFunction.hpp"
#include "intermedia/optimizer/optimizer.hpp"
#include "intermedia/praeCompiler/compiler.hpp"
#include "lexerDriver.hpp"
#include "parser.hpp"
#include "parserTag.hpp"

using namespace frma;
using namespace fie;

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

#ifdef _NDEBUG
  try {
#endif
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

    bool success = !parse.parse() && tag.errors().empty();

    for (auto r : tag.errors()) r.print(std::cerr);

    // if (tag.prims)
    //   std::cout << tag.prims << std::endl;
    // else if (success)
    //   std::cerr << "WARNING: no output" << std::endl;

    if (success && tag.prims) {
      auto compiler  = fnew<FIPraeCompiler>();
      auto optimizer = fnew<FIOptimizer>();
      auto dumpFunc  = fnew<FIDumpFunction>(std::cerr);

      fps::connect<FIFunction>(*compiler, *dumpFunc);
      fps::connect<std::string>(*compiler, *dumpFunc);
      fps::connect<FIMessageId>(*compiler, *dumpFunc);

      auto blocks = compiler->compileBlocks(tag.prims);
    }

    // if (success) {
    //   // FDumbInterpreter interp(tag.prims);

    //   // interp.run();
    // }

    std::cout << std::endl;

    std::cerr << tag.errors().size() << " error";
    if (tag.errors().size() != 1) std::cerr << "s";
    std::cerr << "." << std::endl;

    return tag.errors().size() ? 1 : 0;
#ifdef _NDEBUG
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
#endif
}
