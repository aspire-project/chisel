#!/bin/bash

NAME=for1
SRC=$NAME.c
BIN=$NAME

clang -w -o $BIN $SRC >& /dev/null || exit 1
diff -q <(./$BIN) <(echo "23") >& /dev/null || exit 1
rm $BIN
