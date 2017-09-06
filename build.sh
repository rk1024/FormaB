#!/bin/sh
cd $(dirname "$0")
scripts/build $@
RETCODE=$?
cd $OLDPWD
exit $RETCODE