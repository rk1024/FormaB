#pragma once

#include <sstream>
#include <stack>
#include <vector>

#include "location.hh"

#include "ast.hpp"

namespace frma {
struct FormaParserError {
  frma::location loc;
  std::string    str;

  void print(std::ostream &os) const {
    os << "\x1b[1m";

    if (loc.begin.filename)
      os << *loc.begin.filename;
    else
      os << "???";


    os << ":" << loc.begin.line << ":" << loc.begin.column;

    if (loc.end != loc.begin) os << "-";

    if (loc.end.filename != loc.begin.filename) {
      if (loc.end.filename)
        os << *loc.end.filename;
      else
        os << "???";

      os << ":";

      goto diffLine;
    } else if (loc.end.line != loc.begin.line) {
    diffLine:
      os << loc.end.line << ":";

      goto diffCol;
    } else if (loc.end.column != loc.begin.column) {
    diffCol:
      os << loc.end.column;
    }

    os << ": \x1b[38;5;9merror:\x1b[0m " << str << std::endl;
  }
};

class FormaParserTag {
  std::vector<FormaParserError> m_errors;

  std::string m_filename;

  std::stack<std::ostringstream> bufs;

public:
  FPrims * prims   = nullptr;
  void *   scan    = nullptr;
  bool     lexFail = false;
  location lexFailPos;


  FormaParserTag(const std::string &filename) : m_filename(filename) {}

  ~FormaParserTag() {
    if (prims) delete prims;
  }

  void error(const frma::location &loc, const std::string &str) {
    m_errors.push_back(FormaParserError{.loc = loc, .str = str});
  }

  void error(const frma::location &loc, const char *str) {
    error(loc, std::string(str));
  }

  const std::vector<FormaParserError> errors() const { return m_errors; }

  std::string &filename() { return m_filename; }

  inline void bufStart() { bufs.emplace(); }

  inline void bufEnd() { bufs.pop(); }

  inline void bufReturn() {
    std::string top = bufs.top().str();

    bufs.pop();

    if (!bufs.empty()) bufs.top() << top;
  }

  std::ostringstream &buf() { return bufs.top(); }
};
}
