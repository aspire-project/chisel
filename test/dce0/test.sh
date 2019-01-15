#!/bin/bash

BIN=dce0
SRC=$BIN.c

clang -w -o $BIN $SRC >& /dev/null || exit 1
diff -q <(./$BIN) <(echo "33") >& /dev/null || exit 1
rm $BIN
