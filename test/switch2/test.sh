#!/bin/bash

NAME=switch2
SRC=$NAME.c
BIN=$NAME

clang -w -o $BIN $SRC >& /dev/null || exit 1
diff -q <(timeout 0.1 ./$BIN) <(echo "a") >& /dev/null || exit 1
rm $BIN
