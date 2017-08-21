#!/bin/sh

OLDWD=$PWD
cd "$(dirname $0)"

git submodule update --init --recursive --remote --force
git config --local core.hooksPath scripts/git-hooks
scripts/setup-re2c.sh

cd $OLDWD