/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (parserTag.hpp)
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

#include <sstream>
#include <stack>
#include <string>

#include "diagnostic/location.hpp"
#include "diagnostic/logger.hpp"

#include "ast.hpp"

namespace fps {
struct FParserTag {
  std::string                    m_filename;
  std::stack<std::ostringstream> m_bufs;
  const fdi::FLogger *           m_logger;

public:
  FInputs *      inputs  = nullptr;
  void *         scan    = nullptr;
  bool           lexFail = false;
  fdi::FLocation lexFailPos;

  constexpr auto &buf() const { return m_bufs.top(); }
  constexpr auto &logger() const { return *m_logger; }

  FParserTag(const fdi::FLogger &logger, const std::string &filename) :
      m_filename(filename),
      m_logger(&logger) {}

  ~FParserTag() {
    if (inputs) delete inputs;
  }

  void error(const fdi::FLocation &loc, const std::string &str) {
    m_logger->error(loc, str);
  }

  std::string &filename() { return m_filename; }

  void bufStart() { m_bufs.emplace(); }

  void bufEnd() { m_bufs.pop(); }

  void bufReturn() {
    std::string top = m_bufs.top().str();

    m_bufs.pop();

    if (!m_bufs.empty()) m_bufs.top() << top;
  }
};
} // namespace fps
