#!/usr/bin/zsh
cd $(dirname "$0")
python scripts/build.py $@
RETCODE=$?
cd $OLDPWD
exit $RETCODE