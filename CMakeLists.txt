cmake_minimum_required(VERSION 2.9)
project(chisel)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(LLVM_LINK_COMPONENTS support)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

list(APPEND CMAKE_MODULE_PATH
    ${CMAKE_SOURCE_DIR}/cmake/Modules
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -w -Os -march=native")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lmlpack -larmadillo")

find_package(OpenMP)
if(OPENMP_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif()

find_package(libclang REQUIRED)
include_directories(${LIBCLANG_INCLUDE_DIRS} SYSTEM
    include
    lib
)

add_executable(chisel
    src/chisel.cc
    src/clang-helper/AST_manipulation.cc
    src/clang-helper/cursor_utils.cc
    src/clang-helper/visitor.cc
    src/core/dce.cc
    src/core/rules.cc
    src/core/delta_debugging.cc
    src/core/hierarchical_delta_debugging.cc
    src/learning/model.cc
    src/utils/stats.cc
    src/utils/options.cc
    src/utils/profiling.cc
    src/utils/counting.cc
    src/utils/report.cc
)

target_link_libraries(chisel ${LIBCLANG_LIBRARIES})
