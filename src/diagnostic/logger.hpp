/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (logger.hpp)
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

#pragma once

#include <functional>
#include <ios>

#include "location.hpp"

namespace fdi {
class logger_raise : public std::exception {
  virtual const char *what() const noexcept override { return "logger_raise"; }
};

class FLogger {
public:
  enum Verbosity { Quiet, Normal, Verbose };

private:
  Verbosity             m_verbosity = Normal;
  bool                  m_color     = true;
  std::ostream *        m_os;
  std::function<void()> m_warn, m_err;

public:
  constexpr auto &verbosity() { return m_verbosity; }
  constexpr auto &verbosity() const { return m_verbosity; }

  bool quiet() const { return m_verbosity == Quiet; }
  bool verbose() const { return m_verbosity == Verbose; }
  bool color() const { return m_color; }

  template <typename TWarn, typename TErr>
  FLogger(std::ostream &os, const TWarn &warn, const TErr &err) :
      m_os(&os),
      m_warn(warn),
      m_err(err) {}

private:
  void writeBody(const std::string &lvlFmt,
                 const std::string &lvl,
                 const std::string &str) const {
    if (m_color) *m_os << "\e[m" << lvlFmt;
    *m_os << lvl << ": ";

    if (m_color) *m_os << "\e[m";
    *m_os << str << std::endl;
  }

  void write(const std::string &lvlFmt,
             const std::string &lvl,
             const FLocation &  loc,
             const std::string &str) const {
    if (m_color) *m_os << "\e[1m";
    *m_os << loc << ": ";

    writeBody(lvlFmt, lvl, str);
  }

  void write(const std::string &lvlFmt,
             const std::string &lvl,
             const std::string &pre,
             const std::string &str) const {
    if (m_color) *m_os << "\e[1m";
    *m_os << pre << ": ";

    writeBody(lvlFmt, lvl, str);
  }

  void good(const std::string &lvlFmt,
            const std::string &lvl,
            const FLocation &  loc,
            const std::string &str) const {
    if (!quiet()) write(lvlFmt, lvl, loc, str);
  }

  void good(const std::string &lvlFmt,
            const std::string &lvl,
            const std::string &pre,
            const std::string &str) const {
    if (!quiet()) write(lvlFmt, lvl, pre, str);
  }

  void verb(const std::string &lvlFmt,
            const std::string &lvl,
            const FLocation &  loc,
            const std::string &str) const {
    if (verbose()) write(lvlFmt, lvl, loc, str);
  }

  void verb(const std::string &lvlFmt,
            const std::string &lvl,
            const std::string &pre,
            const std::string &str) const {
    if (verbose()) write(lvlFmt, lvl, pre, str);
  }

  void okay(const std::string &lvlFmt,
            const std::string &lvl,
            const FLocation &  loc,
            const std::string &str) const {
    write(lvlFmt, lvl, loc, str);
    m_warn();
  }

  void okay(const std::string &lvlFmt,
            const std::string &lvl,
            const std::string &pre,
            const std::string &str) const {
    write(lvlFmt, lvl, pre, str);
    m_warn();
  }

  void bad(const std::string &lvlFmt,
           const std::string &lvl,
           const FLocation &  loc,
           const std::string &str) const {
    write(lvlFmt, lvl, loc, str);
    m_err();
  }

  void bad(const std::string &lvlFmt,
           const std::string &lvl,
           const std::string &pre,
           const std::string &str) const {
    write(lvlFmt, lvl, pre, str);
    m_err();
  }

  [[noreturn]] void raise(const std::string &lvlFmt,
                          const std::string &lvl,
                          const FLocation &  loc,
                          const std::string &str) const {
    bad(lvlFmt, lvl, loc, str);
    throw fdi::logger_raise();
  };

  [[noreturn]] void raise(const std::string &lvlFmt,
                          const std::string &lvl,
                          const std::string &pre,
                          const std::string &str) const {
    bad(lvlFmt, lvl, pre, str);
    throw fdi::logger_raise();
  }

  public:;
#define _FLOGFN(name, write, lvl, clr, ...)                                    \
  __VA_ARGS__ void name(const FLocation &loc, const std::string &str) const {  \
    write("\e[1;38;5;" #clr "m", #lvl, loc, str);                              \
  }                                                                            \
  __VA_ARGS__ void name(const std::string &pre, const std::string &str)        \
      const {                                                                  \
    write("\e[1;38;5;" #clr "m", #lvl, pre, str);                              \
  }

  _FLOGFN(debug, good, debug, 8);
  _FLOGFN(info, good, info, 8);
  _FLOGFN(debugV, verb, debug, 8);
  _FLOGFN(infoV, verb, info, 8);
  _FLOGFN(warn, okay, warning, 13);
  _FLOGFN(error, bad, error, 9);
  _FLOGFN(fatal, bad, fatal, 9);
  _FLOGFN(errorR, raise, error, 9, [[noreturn]]);
  _FLOGFN(fatalR, raise, fatal, 9, [[noreturn]]);

#undef _FLOGFN
};
} // namespace fdi
