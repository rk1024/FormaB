/*************************************************************************
 *
 * FormaB - the bootstrap Forma compiler (cli.hpp)
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

#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

namespace fun {
class bad_argument : public std::exception {
  virtual const char *what() const noexcept override { return "bad_argument"; }
};

class FArgParser {
protected:
  enum TakesArg {
    None,
    Optional,
    Required,
  };

  virtual TakesArg takesArg(bool shortFlag, const std::string &flag);

  virtual TakesArg handleVal(bool               shortFlag,
                             const std::string &flag,
                             const std::string &val,
                             int                i);

  virtual void handleArg(const std::string &arg);

public:
  void parse(const std::vector<std::string> &);

  inline void parse(int argc, char **argv) {
    parse(std::vector<std::string>(argv + 1, argv + argc));
  }
};
} // namespace fun
