#!/bin/sh
DIRNAME=$(dirname $0)
BISON=$1
[ "$#" -gt 0 ] && shift
$BISON $@ 2>&1 | "$DIRNAME/colorize-bison.rb"