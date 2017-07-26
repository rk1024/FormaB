#!/bin/sh
DIRNAME=$(dirname $0)
"$DIRNAME/pipe-bison.sh" $@ | "$DIRNAME/colorize-bison.rb"