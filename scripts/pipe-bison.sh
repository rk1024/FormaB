#!/bin/sh
BISON=$1
[ "$#" -gt 0 ] && shift
exec 2>&1
$BISON $@