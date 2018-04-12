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

#include <ios>

#include "location.hpp"

namespace fdi {
class FLogger {
  enum { Quiet, Normal, Verbose } m_verbosity;
  bool          m_color = true;
  std::ostream *m_os;

public:
  bool quiet() const { return m_verbosity == Quiet; }
  bool verbose() const { return m_verbosity == Verbose; }
  bool color() const { return m_color; }

  FLogger(std::ostream &os) : m_os(&os) {}

private:
  void write(const std::string &lvlFmt,
             const std::string &lvl,
             const FLocation &  loc,
             const std::string &str) {
    if (m_color) *m_os << "\e[1m";
    *m_os << loc << ":";

    if (m_color) *m_os << "\e[0m";
    *m_os << " ";

    if (m_color) *m_os << lvlFmt;
    *m_os << lvl << ":";

    if (m_color) *m_os << "\e[0m";
    *m_os << " " << str << std::endl;
  }

public:
  void error(const FLocation &loc, const std::string &str) {
    write("\e[1;38;5;9m", "error", loc, str);
  }
};
} // namespace fdi
