cmake_minimum_required (VERSION 3.1)
project (json)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

add_library(json INTERFACE)
target_include_directories(webview INTERFACE ${PROJECT_SOURCE_DIR})
target_compile_definitions(webview INTERFACE)