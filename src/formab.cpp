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

#include <signal.h>

#include <llvm/Support/raw_ostream.h>

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
  fun::FAtomStore<std::string> m_flags, m_dotModes;
  using Flag    = decltype(m_flags)::Atom;
  using DotMode = decltype(m_dotModes)::Atom;

  std::unordered_map<std::string, Flag> m_short;
  Flag    m_dot, m_help, m_module, m_usage, m_verbose;
  DotMode m_dotDeps;

  std::list<std::string> m_args;
  DotMode                m_dotMode = DotMode(-1);
  std::string            m_moduleName;
  bool                   m_showHelp = false, m_showUsage = false;
  int                    m_verboseLvl = 0;

  Flag resolve(bool shortFlag, const std::string &flag) {
    Flag id;

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
    Flag id = resolve(shortFlag, flag);

    if (id == m_dot)
      return Optional;
    else if (id == m_help)
      m_showHelp = true;
    else if (id == m_module)
      return Required;
    else if (id == m_usage)
      m_showUsage = true;
    else if (id == m_verbose)
      ++m_verboseLvl;

    return None;
  }

  virtual TakesArg handleVal(bool               shortFlag,
                             const std::string &flag,
                             const std::string &val,
                             int) override {
    Flag id = resolve(shortFlag, flag);

    if (id == m_dot) {
      if (!m_dotModes.find(val, &m_dotMode)) {
        std::cerr << "Invalid --dot mode '" << val << "'." << std::endl;
        throw fun::bad_argument();
      }
    }
    else if (id == m_module) {
      m_moduleName = val;
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

  inline auto &args() const { return m_args; }
  inline auto  dotMode() const { return m_dotMode; }
  inline auto  moduleName() const { return m_moduleName; }
  inline auto  showHelp() const { return m_showHelp; }
  inline auto  showUsage() const { return m_showUsage; }
  inline auto  verboseLvl() const { return m_verboseLvl; }

  ArgParser() {
#define MKFLAG(name, lname, sname)                                             \
  m_short.emplace(#sname, m_##name = m_flags.intern(#lname));

    MKFLAG(dot, dot, d)
    MKFLAG(help, help, h)
    MKFLAG(module, module, m)
    MKFLAG(usage, usage, u)
    MKFLAG(verbose, verbose, v)

    m_dotDeps = m_dotModes.intern("deps");
#undef MKFLAG
  }

  void usage() {
    std::cerr << "Usage: formab [flags] [--] [input]" << std::endl;
  }

  void help() {
    usage();

    std::cerr << "Flags:\n"
                 "  \e[1m--verbose, -v\e[0m: Output extra information.\n"
                 "  \e[1m--dot [type], -d [type]\e[0m: Output a dotfile.\n"
                 "    \e[1mTODO:\e[0m document [type]\n"
                 "  \e[1m--help, -h\e[0m: Display this message.\n"
                 "  \e[1m--module [name], -m [name]\e[0m: Specify module "
                 "name.\n"
                 "  \e[1m--usage, -u\e[0m: Display brief usage info.\n";
  }
};

static bool                                      s_normalExit = false;
static std::unordered_map<int, std::string>      s_signals;
static std::unordered_map<int, struct sigaction> s_sigOacts;

void postRun();

void onSignal(int, siginfo_t *, void *);

int run(int, char **);

int main(int argc, char **argv) {
  if (atexit(postRun)) {
    std::cerr << "Failed to register atexit handler." << std::endl;
    return 1;
  }

  s_signals[SIGINT] = "SIGINT";
#if !defined(DEBUG)
  s_signals[SIGILL]  = "SIGILL";
  s_signals[SIGABRT] = "SIGABRT";
  s_signals[SIGFPE]  = "SIGFPE";
  s_signals[SIGSEGV] = "SIGSEGV";
#endif
  s_signals[SIGTERM]   = "SIGTERM";
  s_signals[SIGHUP]    = "SIGHUP";
  s_signals[SIGQUIT]   = "SIGQUIT";
  s_signals[SIGTRAP]   = "SIGTRAP";
  s_signals[SIGBUS]    = "SIGBUS";
  s_signals[SIGSYS]    = "SIGSYS";
  s_signals[SIGPIPE]   = "SIGPIPE";
  s_signals[SIGALRM]   = "SIGALRM";
  s_signals[SIGURG]    = "SIGURG";
  s_signals[SIGTSTP]   = "SIGTSTP";
  s_signals[SIGCONT]   = "SIGCONT";
  s_signals[SIGCHLD]   = "SIGCHLD";
  s_signals[SIGXCPU]   = "SIGXCPU";
  s_signals[SIGXFSZ]   = "SIGXFSZ";
  s_signals[SIGVTALRM] = "SIGVTALRM";
  s_signals[SIGPROF]   = "SIGPROF";

  for (auto pair : s_signals) {
    struct sigaction act, oact;
    act.sa_flags = SA_SIGINFO | SA_NODEFER;
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = onSignal;

    if (sigaction(pair.first, &act, &oact)) {
      std::cerr << "Failed to register " << pair.second << " handler."
                << std::endl;
      return 1;
    }

    s_sigOacts.emplace(pair.first, oact);
  }

  int ret      = run(argc, argv);
  s_normalExit = true;
  return ret;
}

void postRun() {
  if (s_normalExit) {
#if defined(DEBUG)
    std::cerr << "Exited normally." << std::endl;
#endif
    return;
  }

  std::cerr << "WARNING: Exited unexpectedly!" << std::endl;
}

void onSignal(int id, siginfo_t *, void *) {
  std::cerr << "Received " << s_signals[id] << "." << std::endl;

  struct sigaction oact;
  sigaction(id, &s_sigOacts[id], &oact);
  raise(id);
  sigaction(id, &oact, nullptr);
}

int run(int argc, char **argv) {
  FILE *      infile;
  std::string filename("???"), moduleName;

  ArgParser ap;

  try {
    ap.parse(argc, argv);

    if (ap.showHelp()) {
      ap.help();
      return 0;
    }

    if (ap.showUsage()) {
      ap.usage();
      return 0;
    }

    // TODO: This probably needs further validation
    if (ap.moduleName().empty()) {
      std::cerr << "Bad module name." << std::endl;
      throw fun::bad_argument();
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
    default: throw fun::bad_argument();
    }
  }
  catch (fun::bad_argument &) {
    ap.help();
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
      auto inputs = fnew<fie::FIInputs>(assem, moduleName, filename);

      auto sched = fnew<fie::FIScheduler>(graph, inputs);

      sched->schedule(tag.prims);

      if (ap.dotMode() == ap.dotDeps())
        graph->dot(std::cout);
      else {
        try {
          graph->run();

          inputs->llModule()->print(llvm::errs(), nullptr);
        }
        catch (fun::compiler_error &) {
          assert(tag.errors().size());
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
