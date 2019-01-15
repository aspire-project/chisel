#!/bin/bash

BIN=conditional2
SRC=$BIN.c

clang -w -o $BIN $SRC >& /dev/null || exit 1
diff -q <(./$BIN) <(echo "15") >& /dev/null || exit 1
rm $BIN
