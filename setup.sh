#!/bin/sh

OLDWD=$PWD
cd "$(dirname $0)"

bundle lock
git submodule update --recursive --remote
git config --local core.hooksPath scripts/git-hooks
scripts/setup-re2c.sh

echo "\n"

echo "\033[1;38;5;1mNOTE:\033[39m To build formab, run ./build.sh\033[0m"
echo "\033[1;38;5;1mNOTE:\033[39m The build script will probably fail the first time you run it - just try again.\033[0m"

echo "\n"

cd $OLDWD