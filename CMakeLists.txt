cmake_minimum_required(VERSION 2.9)
project(chisel)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

set(CMAKE_CXX_FLAGS "${LLVM_COMPILE_FLAGS} ${CMAKE_CXX_FLAGS} -std=c++11 -lstdc++ -w -Os -march=native -fexceptions -pthread")
if(CMAKE_BUILD_TYPE MATCHES DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address")
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

message(STATUS "CXX flags: " ${CMAKE_CXX_FLAGS})

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(include)

add_executable(chisel
  src/Chisel.cpp
  src/core/Frontend.cpp
  src/core/SourceManager.cpp
  src/core/Transformation.cpp
  src/core/Reduction.cpp
  src/core/GlobalReduction.cpp
  src/core/LocalReduction.cpp
  src/core/DeadcodeElimination.cpp
  src/core/Reformat.cpp
  src/core/ProbabilisticModel.cpp
  src/utils/FileManager.cpp
  src/utils/OptionManager.cpp
  src/utils/Report.cpp
  src/utils/Profiler.cpp
  src/utils/StatsManager.cpp
)

target_link_libraries(chisel ${CLANG_LIBS} ${LLVM_LIBS_CORE} ${LLVM_LDFLAGS})

# for make test
enable_testing()
add_subdirectory(test)

# for documents
option(BUILD_DOCS "Build and Install Documents (Requires Doxygen)")
if (BUILD_DOCS)
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    # note the option ALL which allows to build the docs together with the application
    add_custom_target(doc ALL
      COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Generating API documentation with Doxygen"
      VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)
endif(BUILD_DOCS)
