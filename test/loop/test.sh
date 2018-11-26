#!/bin/bash
BIN=loop
SRC=$BIN.c

clang -w -o $BIN $SRC >& /dev/null || exit 1
timeout 0.1 ./$BIN > log
diff -q input log >& /dev/null || exit 1
rm $BIN log
