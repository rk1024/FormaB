#!/bin/sh

OLDWD=$PWD
cd "$(dirname $0)"

git submodule update --recursive --remote --init
git config --local core.hooksPath scripts/git-hooks

echo

echo "\033[1;38;5;1mNOTE:\033[39m To build formab, run ./build.sh\033[0m"
echo "\033[1;38;5;1mNOTE:\033[39m The build script will probably fail the first time you run it - just try again.\033[0m"

echo

cd $OLDWD