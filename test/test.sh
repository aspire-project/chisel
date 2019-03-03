#!/bin/bash

CHISEL_HOME=$(cd $(dirname ${BASH_SOURCE[0]}) && cd ../ && pwd)
CHISEL=$CHISEL_HOME/build/bin/chisel
TEST_NAME=$1

for ARG in "${@:2}"; do
  if [[ $ARG == --* ]]; then
    OPTIONS="$OPTIONS $ARG"
  else
    SRC_FILES="$SRC_FILES $ARG"
  fi
done

cd $CHISEL_HOME/test/$TEST_NAME

rm -rf $TEST_NAME $TEST_NAME.c $TEST_NAME.c.chisel.c chisel-out *.o
cp $TEST_NAME.c.orig.c $TEST_NAME.c

SRC_FILES_ARRAY=($SRC_FILES)
for SRC in "${SRC_FILES_ARRAY[@]}"; do
  rm -rf $SRC $SRC.chisel.c
  cp $SRC.orig.c $SRC
done

$CHISEL $OPTIONS ./test.sh $TEST_NAME.c $SRC_FILES

rm -rf $TEST_NAME $TEST_NAME.c chisel-out *.o
for SRC in "${SRC_FILES_ARRAY[@]}"; do
  rm -rf $SRC
done

diff -q $TEST_NAME.c.chisel.c $TEST_NAME.c.answer.c || exit 1
for SRC in "${SRC_FILES_ARRAY[@]}"; do
  diff -q $SRC.chisel.c $SRC.answer.c || exit 1
done
