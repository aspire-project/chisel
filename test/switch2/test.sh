#!/bin/bash

NAME=switch2
SRC=$NAME.c
BIN=$NAME

clang -w -o $BIN $SRC >& /dev/null || exit 1
diff -q <(./$BIN) <(echo "a") >& /dev/null || exit 1
rm $BIN
