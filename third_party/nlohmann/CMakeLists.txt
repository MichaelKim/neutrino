cmake_minimum_required (VERSION 3.1)
project (webkit)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

add_library(json ${CMAKE_CURRENT_BINARY_DIR}/json.cpp)
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/json.cpp "#include <json.hpp>")
target_include_directories(json PUBLIC ${PROJECT_SOURCE_DIR})
target_compile_definitions(json PUBLIC)