#!/bin/bash

BIN=do-while1
SRC=$BIN.c

clang -w -o $BIN $SRC >& /dev/null || exit 1
timeout 0.1 ./$BIN || exit 1
rm $BIN
