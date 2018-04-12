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
#include <vector>

#include <signal.h>

#include "util/atom.hpp"
#include "util/cli.hpp"

#include "diagnostic/logger.hpp"

#include "parser.hpp"
#include "parser/lexerDriver.hpp"
#include "parser/parserTag.hpp"

class FMainArgParser : public fun::FArgParser {
  fun::FAtomStore<std::string> m_flags;
  using Flag = decltype(m_flags)::Atom;

  std::unordered_map<std::string, Flag> m_short;
  Flag                                  m_help, m_usage, m_verbose;

  std::vector<std::string> m_args;
  bool                     m_showHelp = false, m_showUsage = false;
  int                      m_verboseLvl = 0;

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
    // TODO: Add diagnostic logging
    std::cerr << "Invalid flag '" << flag << "'." << std::endl;
    throw fun::bad_argument();
  }

  virtual TakesArg takesArg(bool shortFlag, const std::string &flag) override {
    Flag id = resolve(shortFlag, flag);

    if (id == m_help)
      m_showHelp = true;
    else if (id == m_usage)
      m_showUsage = true;
    else if (id == m_verbose)
      ++m_verboseLvl;

    return None;
  }

  virtual TakesArg handleVal(bool /* shortFlag */,
                             const std::string & /* flag */,
                             const std::string & /* val */,
                             int) override {
    // Flag id = resolve(shortFlag, flag);

    return None;
  }

  virtual void handleArg(const std::string &arg) override {
    m_args.push_back(arg);
  }

public:
  constexpr auto &args() const { return m_args; }
  constexpr auto &showHelp() const { return m_showHelp; }
  constexpr auto &showUsage() const { return m_showUsage; }
  constexpr auto &verboseLvl() const { return m_verboseLvl; }

  FMainArgParser() {
#define MKFLAG(name, lname, sname)                                             \
  m_short.emplace(#sname, m_##name = m_flags.intern(#lname));

    MKFLAG(help, help, h)
    MKFLAG(usage, usage, u)
    MKFLAG(verbose, verbose, v)

#undef MKFLAG
  }

  void usage() {
    std::cerr << "Usage: formab [flags] [--] [input]" << std::endl;
  }

  void help() {
    usage();

    // TODO: Find a better solution for this
    std::cerr << "Flags:\n"
                 "  \e[1m--verbose, -v\e[0m: Output extra information.\n"
                 "  \e[1m--help, -h\e[0m: Display this message.\n"
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
    return -1;
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
  std::cerr << "Received ";
  if (auto it = s_signals.find(id); it != s_signals.end())
    std::cerr << it->second;
  else
    std::cerr << "signal " << id;
  std::cerr << "." << std::endl;

  struct sigaction oact;
  sigaction(id, &s_sigOacts[id], &oact);
  raise(id);
  sigaction(id, &oact, nullptr);
}

int run(int argc, char **argv) {
  fdi::FLogger logger(std::cerr);

  FILE *      infile;
  std::string filename("???");

  FMainArgParser ap;

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

    auto &&args = ap.args();

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
    fps::FParserTag tag(logger, filename);
    fps::FLexer     lex(tag);
    fps::parser     parse(&tag, &lex);

    lex.inFile(infile);

#if defined(DEBUG)
    if (ap.verboseLvl() > 1) {
      lex.debug(true);
      parse.set_debug_level(1);
    }
#endif

    bool success = !parse.parse() && true; // TODO: Add proper error handling

    return 0;
#if defined(NDEBUG)
  }
  catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
    return -1;
  }
#endif
}
