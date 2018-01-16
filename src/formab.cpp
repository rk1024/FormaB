/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (formab.cpp)
 * Copyright (C) 2017-2018 Ryan Schroeder, Colin Unger
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
 ************************************************************************/

#include <cstdio>
#include <list>

#include "pipeline/depsGraph.hpp"

#include "lexerDriver.hpp"
#include "parser.hpp"
#include "parserTag.hpp"

#include "ast/walker.hpp"

#include "util/atom.hpp"
#include "util/cli.hpp"
#include "util/compilerError.hpp"

#include "intermedia/debug/dumpFunction.hpp"
#include "intermedia/optimizer/optimizer.hpp"
#include "intermedia/praeCompiler/compiler.hpp"
#include "intermedia/scheduler.hpp"
#include "intermedia/verifier/verifier.hpp"

using namespace frma;
using namespace fie;

class ArgParser : public fun::FArgParser {
  fun::FAtomStore<std::string>                 m_flags, m_dot_modes;
  std::unordered_map<std::string, std::size_t> m_short;
  std::size_t m_verbose, m_dot, m_dotDeps, m_dotControl;

  std::list<std::string> m_args;
  std::size_t            m_dotMode    = -1;
  int                    m_verboseLvl = 0;

  std::size_t resolve(bool shortFlag, const std::string &flag) {
    std::size_t id;

    if (shortFlag) {
      auto it = m_short.find(flag);
      if (it == m_short.end()) goto nonexist;
      id = it->second;
    }
    else if (!m_flags.find(flag, &id))
      goto nonexist;

    return id;

  nonexist:
    std::cerr << "Invalid flag '" << flag << "'." << std::endl;
    throw fun::bad_argument();
  }

  virtual TakesArg takesArg(bool shortFlag, const std::string &flag) override {
    std::size_t id = resolve(shortFlag, flag);

    if (id == m_verbose) ++m_verboseLvl;

    if (id == m_dot)
      return Optional;
    else
      return None;
  }

  virtual TakesArg handleVal(bool               shortFlag,
                             const std::string &flag,
                             const std::string &val,
                             int) override {
    std::size_t id = resolve(shortFlag, flag);

    if (id == m_dot) {
      if (!m_dot_modes.find(val, &m_dotMode)) {
        std::cerr << "Invalid --dot mode '" << val << "'." << std::endl;
        throw fun::bad_argument();
      }
    }
    else
      assert(false);

    return None;
  }

  virtual void handleArg(const std::string &arg) override {
    m_args.push_back(arg);
  }

public:
  inline auto dotDeps() const { return m_dotDeps; }
  inline auto dotControl() const { return m_dotControl; }

  inline auto  dotMode() const { return m_dotMode; }
  inline auto  verboseLvl() const { return m_verboseLvl; }
  inline auto &args() const { return m_args; }

  ArgParser() {
#define MKFLAG(lname, sname)                                                   \
  m_short.emplace(#sname, m_##lname = m_flags.intern(#lname));

    MKFLAG(verbose, v)
    MKFLAG(dot, d)

    m_dotDeps    = m_dot_modes.intern("deps");
    m_dotControl = m_dot_modes.intern("control");
#undef MKFLAG
  }

  void help() {
    std::cerr << "Usage: formab [flags] [--] [input]" << std::endl;

    std::cerr << "Flags:\n  \e[1m--verbose, -v\e[0m: Output extra information."
              << std::endl
              << "  \e[1m--dot, -d\e[0m: Output dependency graph as a dotfile."
              << std::endl;
  }
};

int main(int argc, char **argv) {
  FILE *      infile;
  std::string filename("???");

  ArgParser ap;

  try {
    ap.parse(argc, argv);
  }
  catch (fun::bad_argument &) {
    exit(1);
  }

  std::vector<std::string> args(ap.args().begin(), ap.args().end());

  switch (args.size()) {
  case 0:
  readStdin:
    infile   = stdin;
    filename = "<stdin>";
    break;
  case 1:
    if (args.at(0) == "-") goto readStdin;
    infile   = fopen(args.at(0).c_str(), "r");
    filename = args.at(0);
    break;
  default: ap.help(); return 1;
  }

#if defined(NDEBUG)
  try {
#endif
    frma::FormaParserTag tag(filename);
    frma::lexer          lex(tag);
    frma::parser         parse(&tag, &lex);

    lex.inFile(infile);

#if defined(DEBUG)
    if (ap.verboseLvl() > 1) {
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

      if (ap.dotMode() == ap.dotDeps())
        graph->dot(std::cout);
      else {
        try {
          graph->run();
        }
        catch (fun::compiler_error &) {
          return 1;
        }
      }
    }

    std::cerr << tag.errors().size() << " error";
    if (tag.errors().size() != 1) std::cerr << "s";
    std::cerr << "." << std::endl;

    return tag.errors().size() ? 1 : 0;
#if defined(NDEBUG)
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
#endif
}
