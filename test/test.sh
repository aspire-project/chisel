#!/bin/bash

CHISEL_HOME=$(cd $(dirname ${BASH_SOURCE[0]}) && cd ../ && pwd)
CHISEL=$CHISEL_HOME/build/bin/chisel
TEST_NAME=$1
ARGS="${@:2}"

cd $CHISEL_HOME/test/$TEST_NAME

rm -rf $TEST_NAME $TEST_NAME.c.chisel.c chisel-out *.o

$CHISEL $OPTIONS ./test.sh $ARGS

rm -rf $TEST_NAME *.o

for SRC in $ARGS; do
  if [[ $SRC == *.c ]]; then
    diff -q $SRC.chisel.c $SRC.answer.c || exit 1
  fi
done

rm -rf *.chisel.c *.origin.c chisel-out
