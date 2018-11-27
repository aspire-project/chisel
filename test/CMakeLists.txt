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
