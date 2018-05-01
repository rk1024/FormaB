/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (lexerDriver.hpp)
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

#include "parserTag.hpp"

namespace fps {
class FLexer {
  void *              m_yyscanner;
  const fdi::FLogger *m_logger;

public:
  FLexer(FParserTag &);

  ~FLexer();

  void init();

#if defined(_DEBUG)
  bool debug() const;

  void debug(bool);
#endif

  FILE *inFile() const;

  void inFile(FILE *);

  FILE *outFile() const;

  void outFile(FILE *);
};
} // namespace fps
