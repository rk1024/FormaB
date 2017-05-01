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
    if (loc.begin.filename)
      os << *loc.begin.filename;
    else
      os << "???";


    os << ":" << loc.begin.line << ":" << loc.begin.column << ": " << str
       << std::endl;
  }
};

class FormaParserTag {
  std::vector<FormaParserError> m_errors;

  std::string m_filename;

  std::stack<std::ostringstream> bufs;

public:
  FormaPrims *prims = nullptr;
  void *      scan  = nullptr;


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
