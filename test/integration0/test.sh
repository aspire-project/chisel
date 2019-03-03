#!/bin/bash

NAME=integration0
SRC=$NAME.c
BIN=$NAME

clang -w -c subfile0.c >&/dev/null || exit 1
clang -w -c subfile1.c >&/dev/null || exit 1
clang -w -o $BIN $SRC subfile0.o subfile1.o >&/dev/null || exit 1
./$BIN >&/dev/null || exit 1
rm $BIN
