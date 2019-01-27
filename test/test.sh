#!/bin/bash

CHISEL_HOME=$(cd $(dirname ${BASH_SOURCE[0]}) && cd ../ && pwd)
CHISEL=$CHISEL_HOME/build/bin/chisel
TEST_NAME=$1
OPTIONS="${@:2}"

cd $CHISEL_HOME/test/$TEST_NAME

rm -rf $TEST_NAME $TEST_NAME.c $TEST_NAME.c.chisel.c chisel-out
cp $TEST_NAME.c.orig.c $TEST_NAME.c
$CHISEL $OPTIONS ./test.sh $TEST_NAME.c
rm -rf $TEST_NAME $TEST_NAME.c chisel-out
diff -q $TEST_NAME.c.chisel.c $TEST_NAME.c.answer.c
