add_test(
  NAME basic0
  COMMAND ./test.sh basic0 basic0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME basic1
  COMMAND ./test.sh basic1 basic1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME basic2
  COMMAND ./test.sh basic2 basic2.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME basic3
  COMMAND ./test.sh basic3 basic3.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME basic4
  COMMAND ./test.sh basic4 basic4.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME basic5
  COMMAND ./test.sh basic5 basic5.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME conditional0
  COMMAND ./test.sh conditional0 conditional0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME conditional1
  COMMAND ./test.sh conditional1 conditional1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME conditional2
  COMMAND ./test.sh conditional2 conditional2.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME conditional3
  COMMAND ./test.sh conditional3 conditional3.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME loop0
  COMMAND ./test.sh loop0 loop0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME loop1
  COMMAND ./test.sh loop1 loop1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME for0
  COMMAND ./test.sh for0 for0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME for1
  COMMAND ./test.sh for1 for1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME for2
  COMMAND ./test.sh for2 for2.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME for3
  COMMAND ./test.sh for3 for3.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME do-while0
  COMMAND ./test.sh do-while0 do-while0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME do-while1
  COMMAND ./test.sh do-while1 do-while1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME do-while2
  COMMAND ./test.sh do-while2 do-while2.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME switch0
  COMMAND ./test.sh switch0 switch0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME switch1
  COMMAND ./test.sh switch1 switch1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME switch2
  COMMAND ./test.sh switch2 switch2.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME switch3
  COMMAND ./test.sh switch3 switch3.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME function0
  COMMAND ./test.sh function0 function0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME function1
  COMMAND ./test.sh function1 function1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME dce0
  COMMAND ./test.sh dce0 dce0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME dce1
  COMMAND ./test.sh dce1 dce1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME dce2
  COMMAND ./test.sh dce2 dce2.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME dce3
  COMMAND ./test.sh dce3 dce3.c --skip_global --skip_local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME dce4
  COMMAND ./test.sh dce4 dce4.c --skip_global --skip_local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME label0
  COMMAND ./test.sh label0 label0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME macro0
  COMMAND ./test.sh macro0 macro0.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME macro1
  COMMAND ./test.sh macro1 --build -- gcc -DMACRO=0
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME integration0
  COMMAND ./test.sh integration0 integration0.c subfile0.c subfile1.c
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME integration1
  COMMAND ./test.sh integration1 --build -- make
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)
