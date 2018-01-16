/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (cli.re)
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

#include "cli.hpp"

#include <cassert>

#include <iostream>

#include "util/ptr.hpp"

namespace fun {
FArgParser::TakesArg FArgParser::takesArg(bool, const std::string &) {
  return None;
}

FArgParser::TakesArg FArgParser::handleVal(bool,
                                           const std::string &,
                                           const std::string &,
                                           int) {
  return None;
}

void FArgParser::handleArg(const std::string &) {}

void FArgParser::parse(const std::vector<std::string> &args) {
  auto                         it = args.begin(), end = args.end();
  std::pair<bool, std::string> acceptFlag;
  int                          acceptIdx;
  TakesArg                     acceptMode = TakesArg::None;

  for (; it != end; ++it) {
    auto &      arg  = *it;
    const char *_str = arg.c_str(), *str = _str, *YYMARKER, *l0 = nullptr,
               *l1 = nullptr, *l2 = nullptr, *v0 = nullptr, *v1 = nullptr
        /*!stags:re2c format = ", *@@"; */;

    switch (acceptMode) {
    case Required:
      goto handle; // Parsing not necessary; we're just going to handleVal
    default: break;
    }

    /*!re2c
      re2c:flags:T = 1;

      re2c:define:YYCTYPE = char;
      re2c:define:YYCURSOR = str;

      re2c:define:YYSTAGN = "@@ = nullptr";

      re2c:yyfill:enable = 0;

      eof = "\x00";

      dash = "-";
      ddash = dash dash;
      letters = @l0 [a-zA-Z0-9][a-zA-Z0-9-]* @l1;
      nonletter = [^a-zA-Z0-9-\x00];
      eq = "=";
      val = [^\x00]*;

      short = dash letters @l2;
      shortval = dash letters nonletter val @l2;
      long = ddash letters;
      longval = ddash letters eq @v0 val @v1;

      * {
        goto handle;
      }

      long eof {
        goto handle;
      }

      longval eof {
        goto handle;
      }

      short eof {
        goto handle;
      }

      shortval eof {
        goto handle;
      }

      ddash eof {
        ++it;
        break;
      }
    */

  handle:
    switch (acceptMode) {
    case None:
      if (l0)
        goto handleFlag;
      else
        goto handleArg;
    case Optional:
      if (l0) {
        acceptMode = None;
        goto handleFlag;
      } else
        goto handleVal;
    case Required: goto handleVal;
    }

  handleFlag : {
    std::string name;
    if (l2)
      name = std::string(l0, 1);
    else
      name = std::string(l0, l1);

    TakesArg    ta = takesArg(l2, name);
    const char *rest;
    if (l2) rest = l0 + 1;

    if (ta == None) {
      if (v0 || l2 && rest == l1 && l1 != l2) {
        std::cerr << "Flag '" << name << "' does not take a value."
                  << std::endl;
        throw bad_argument();
      }
    } else {
      acceptFlag = std::make_pair(l2, name);
      acceptIdx  = 0;
      acceptMode = ta;

      if (l2) {
        if (rest < l1) {
          std::string val(rest, l1);
          acceptMode = handleVal(l2, name, val, acceptIdx);
          ++acceptIdx;
          rest = l1;
        }
      } else if (v0) {
        std::string val(v0, v1);
        acceptMode = handleVal(l2, name, val, acceptIdx);
        ++acceptIdx;
      }
    }

    if (l2 && rest < l1) {
      l0 = rest;
      goto handleFlag;
    }

    goto done;
  }

  handleVal : {
    acceptMode = handleVal(acceptFlag.first, acceptFlag.second, arg, acceptIdx);
    ++acceptIdx;
    goto done;
  }

  handleArg : {
    handleArg(arg);
    goto done;
  }

  done:;
  }

  if (acceptMode == TakesArg::Required) {
    std::cerr << "Missing required argument for '" << acceptFlag.second << "'."
              << std::endl;
    throw bad_argument();
  }

  for (; it != end; ++it) {
    auto &arg = *it;

    handleArg(arg);
  }
}
} // namespace fun
