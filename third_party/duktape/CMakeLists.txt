cmake_minimum_required (VERSION 3.1)
project (duktape)

add_library(duktape STATIC duktape.c)
target_include_directories(duktape PUBLIC ${PROJECT_SOURCE_DIR})
target_link_libraries(duktape m)