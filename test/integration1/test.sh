#!/bin/bash

NAME=integration1
SRC=$NAME.c
BIN=$NAME

make >&/dev/null || exit 1
./$BIN >&/dev/null || exit 1
make clean >&/dev/null
