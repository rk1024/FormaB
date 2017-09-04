#!/bin/sh

cd tools/re2c/re2c

./autogen.sh
./configure
make bootstrap -j 16

cd ../../..