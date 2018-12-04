add_test(
  NAME basic0
  COMMAND ./test.sh basic0
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME basic1
  COMMAND ./test.sh basic1
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME basic2
  COMMAND ./test.sh basic2
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME conditional0
  COMMAND ./test.sh conditional0
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME conditional1
  COMMAND ./test.sh conditional1
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME conditional2
  COMMAND ./test.sh conditional2
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME loop0
  COMMAND ./test.sh loop0
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

add_test(
  NAME loop1
  COMMAND ./test.sh loop1
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)
