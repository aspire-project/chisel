#!/bin/bash

BIN=function1
SRC=$BIN.c

clang -w -o $BIN $SRC >& /dev/null || exit 1
diff -q <(./$BIN) <(echo "7") >& /dev/null || exit 1
rm $BIN
