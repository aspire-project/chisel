#!/bin/bash

NAME=macro1
SRC=$NAME.c
BIN=$NAME

clang -w -o $BIN -DMACRO=1 $SRC >&/dev/null || exit 1
timeout 0.1 ./$BIN >&/dev/null || exit 1
rm $BIN
