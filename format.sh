#!/bin/sh

scripts/git-clang-format --style=$(git rev-parse --show-toplevel)/.clang-format --staged