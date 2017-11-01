/*************************************************************************
*
* FormaB - the bootstrap Forma compiler (formab.cpp)
* Copyright (C) 2017-2017 Ryan Schroeder, Colin Unger
*
* FormaB is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License as
* published by the Free Software Foundation, either version 3 of the
* License, or (at your option) any later version.
*
* FormaB is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with FormaB.  If not, see <https://www.gnu.org/licenses/>.
*
*************************************************************************/

#include <cstdio>
#include <list>

#include "pipeline/depsGraph.hpp"

#include "lexerDriver.hpp"
#include "parser.hpp"
#include "parserTag.hpp"

#include "ast/walker.hpp"

#include "intermedia/debug/dumpFunction.hpp"
#include "intermedia/optimizer/optimizer.hpp"
#include "intermedia/praeCompiler/compiler.hpp"
#include "intermedia/scheduler.hpp"
#include "intermedia/verifier/verifier.hpp"

using namespace frma;
using namespace fie;

int main(int argc, char **argv) {
  FILE *      infile;
  std::string filename("???");
  bool        dot = false;
#if defined(DEBUG)
  bool verbose = false;
#endif

  std::list<std::string> args;

  {
    bool doFlags = true;
    for (size_t i = 1; i < argc; ++i) {
      std::string arg(argv[i]);
      if (doFlags) {
#if defined(DEBUG)
        if (arg == "--verbose" || arg == "-v")
          verbose = true;
        else
#endif
            if (arg == "--dot" || arg == "-d")
          dot = true;
        else if (arg == "--")
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
    std::cerr << "Usage: " << argv[0] << "[flags] [--] [input]" << std::endl;

    std::cerr << "Flags:\n  \e[1m--verbose, -v\e[0m: Output extra information."
              << std::endl
              << "  \e[1m--dot, -d\e[0m: Output dependency graph as a dotfile."
              << std::endl;
    return 1;
  }

#if defined(NDEBUG)
  try {
#endif
    frma::FormaParserTag tag(filename);
    frma::lexer          lex(tag);
    frma::parser         parse(&tag, &lex);

    lex.inFile(infile);

#if defined(DEBUG)
    if (verbose) {
      lex.debug(true);
      parse.set_debug_level(1);
    }
#endif

    bool success = !parse.parse() && tag.errors().empty();

    for (auto r : tag.errors()) r.print(std::cerr);

    assert(tag.prims || !success);

    if (success && tag.prims) {
      auto graph = fnew<fps::FDepsGraph>();

      auto assem  = fnew<fie::FIAssembly>();
      auto inputs = fnew<fie::FIInputs>(assem);

      auto sched = fnew<fie::FIScheduler>(graph, inputs);

      sched->schedule(tag.prims);

      if (dot)
        graph->dot(std::cout);
      else
        graph->run();
    }

    std::cerr << tag.errors().size() << " error";
    if (tag.errors().size() != 1) std::cerr << "s";
    std::cerr << "." << std::endl;

    return tag.errors().size() ? 1 : 0;
#if defined(NDEBUG)
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
#endif
}
