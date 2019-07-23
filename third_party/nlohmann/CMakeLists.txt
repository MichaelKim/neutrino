cmake_minimum_required (VERSION 3.8)
project (json)

add_library(json INTERFACE)
target_compile_features(json INTERFACE cxx_std_11)
target_include_directories(json INTERFACE ${PROJECT_SOURCE_DIR})