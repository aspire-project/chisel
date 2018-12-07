#!/bin/bash

BIN=function0
SRC=$BIN.c

clang -w -o $BIN $SRC >& /dev/null || exit 1
diff -q <(timeout 0.1 ./$BIN) <(echo "7") >& /dev/null || exit 1
rm $BIN
