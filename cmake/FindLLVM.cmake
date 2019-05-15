# Detect LLVM and set various variable to link against the different component of LLVM
#
# NOTE: This is a modified version of the module originally found in the OpenGTL project
# at www.opengtl.org
#
# LLVM_BIN_DIR : directory with LLVM binaries
# LLVM_LIB_DIR : directory with LLVM library
# LLVM_INCLUDE_DIR : directory with LLVM include
#
# LLVM_COMPILE_FLAGS : compile flags needed to build a program using LLVM headers
# LLVM_LDFLAGS : ldflags needed to link
# LLVM_LIBS_CORE : ldflags needed to link against a LLVM core library
# LLVM_LIBS_JIT : ldflags needed to link against a LLVM JIT
# LLVM_LIBS_JIT_OBJECTS : objects you need to add to your source when using LLVM JIT

set(CONFIG_NAME "llvm-config")
if (NOT "${LLVM_CONFIG_EXECUTABLE}" STREQUAL "")
  set(CONFIG_NAME "${LLVM_CONFIG_EXECUTABLE}")
  unset(LLVM_CONFIG_EXECUTABLE CACHE)
endif()
find_program(LLVM_CONFIG_EXECUTABLE NAMES "${CONFIG_NAME}" "${CONFIG_NAME}-8")

if (BUILD_STATIC)
  set(LLVM_BUILD_OPT "--link-static")
else (BUILD_STATIC)
  set(LLVM_BUILD_OPT "--link-shared")
endif (BUILD_STATIC)

exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --bindir OUTPUT_VARIABLE LLVM_BIN_DIR)
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --libdir OUTPUT_VARIABLE LLVM_LIB_DIR)
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --includedir OUTPUT_VARIABLE LLVM_INCLUDE_DIR)
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --cxxflags  OUTPUT_VARIABLE LLVM_COMPILE_FLAGS)
MESSAGE(STATUS "LLVM CXX flags: " ${LLVM_COMPILE_FLAGS})
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --ldflags OUTPUT_VARIABLE LLVM_LDFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
execute_process(COMMAND ${LLVM_CONFIG_EXECUTABLE} --system-libs ${LLVM_BUILD_OPT} OUTPUT_VARIABLE LLVM_LDFLAGS2 OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
string(REPLACE "\n" " " LLVM_LDFLAGS "${LLVM_LDFLAGS} ${LLVM_LDFLAGS2}")
string(STRIP ${LLVM_LDFLAGS} LLVM_LDFLAGS)
MESSAGE(STATUS "LLVM LD flags: " ${LLVM_LDFLAGS})
exec_program(${LLVM_CONFIG_EXECUTABLE} ARGS --libs ${LLVM_BUILD_OPT} OUTPUT_VARIABLE LLVM_LIBS_CORE)
MESSAGE(STATUS "LLVM core libs: " ${LLVM_LIBS_CORE})

if(LLVM_INCLUDE_DIR)
  set(LLVM_FOUND TRUE)
endif(LLVM_INCLUDE_DIR)

if(LLVM_FOUND)
  message(STATUS "Found LLVM")
else(LLVM_FOUND)
  if(LLVM_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find LLVM")
  endif(LLVM_FIND_REQUIRED)
endif(LLVM_FOUND)
