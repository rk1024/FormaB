#!/bin/sh

git submodule update --init --recursive --remote --force
git config --local core.hooksPath scripts/git-hooks