cmake_minimum_required(VERSION 2.9)
project(chisel)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

set(CMAKE_CXX_FLAGS "${LLVM_COMPILE_FLAGS} -std=c++11 -lstdc++ -w -Os -march=native -fexceptions")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lmlpack -larmadillo")

#find_package(OpenMP)
#if(OPENMP_FOUND)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
#endif()

include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
include_directories(include)

add_executable(chisel
  src/Chisel.cpp
  src/core/Frontend.cpp
  src/core/Transformation.cpp
  src/core/Reduction.cpp
  src/core/GlobalReduction.cpp
  src/core/LocalReduction.cpp
  src/core/DeadcodeElimination.cpp
  src/utils/FileManager.cpp
  src/utils/OptionManager.cpp
  src/utils/Report.cpp
  src/utils/Profiler.cpp
  src/utils/StatsManager.cpp
)

target_link_libraries(chisel ${CLANG_LIBS} ${LLVM_LIBS_CORE} ${LLVM_LDFLAGS})

enable_testing()
add_subdirectory(test)
